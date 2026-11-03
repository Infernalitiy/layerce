"""Real Git commands against isolated local repositories; never GitHub."""
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

source = Path(__file__).resolve().parents[1] / 'tools/auto_sync.py'
spec = importlib.util.spec_from_file_location('auto_sync', source)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class SyncTests(unittest.TestCase):
    def setUp(self):
        temp_root = source.parents[1] / 'test-bin' / 'sync-tests'
        temp_root.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix='case-', dir=temp_root)
        assert Path(self.temp.name).resolve().is_relative_to(temp_root.resolve())
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / 'work'
        self.root.mkdir()
        self.origin = self.base / 'remote.git'
        subprocess.run(['git','init','--bare',str(self.origin)],check=True,capture_output=True)
        self.git('init','-b','dev')
        self.git('config','user.name','Local test')
        self.git('config','user.email','test@example.invalid')
        (self.root / 'README.md').write_text('baseline\n')
        self.git('add','README.md'); self.git('commit','-m','baseline')
        self.git('remote','add','origin',str(self.origin))
        self.config = {'branch':'dev','remote':'origin','remote_url':str(self.origin),'quiet_seconds':0}
        self.backup = module.Backup(self.root, self.config)

    def git(self,*args):
        p = subprocess.run(['git',*args],cwd=self.root,capture_output=True,text=True)
        if p.returncode: raise AssertionError(p.stderr)
        return p.stdout.strip()

    def require_push(self):
        p = subprocess.run(['git','push','-u','origin','dev'],cwd=self.root,capture_output=True,text=True)
        if p.returncode and 'NtCreateDirectoryObject' in p.stderr:
            self.skipTest('Git for Windows local transport blocked by execution sandbox')
        if p.returncode: self.fail(p.stderr)

    def test_saved_code_pushed_and_rom_excluded(self):
        self.require_push()
        (self.root / 'src').mkdir()
        (self.root / 'src/main.c').write_text('int x;\n')
        (self.root / 'dump.rom').write_bytes(b'private ROM placeholder')
        (self.root / '.env').write_text('SECRET=placeholder\n')
        self.assertIn('Backed up',self.backup.cycle())
        names = self.git('ls-tree','-r','--name-only','HEAD').splitlines()
        self.assertIn('src/main.c',names)
        self.assertNotIn('dump.rom',names)
        self.assertNotIn('.env',names)
        self.assertEqual(self.git('rev-parse','HEAD'),self.git('rev-parse','origin/dev'))
        self.assertIn('[skip ci]',self.git('log','-1','--format=%s'))
        self.assertIn('Up to date',self.backup.cycle())

    def test_dry_run_does_not_commit(self):
        (self.root / 'README.md').write_text('changed\n')
        before = self.git('rev-parse','HEAD')
        self.assertIn('README.md',self.backup.cycle(dry_run=True))
        self.assertEqual(before,self.git('rev-parse','HEAD'))

    def test_local_commit_retained_when_network_fails(self):
        (self.root / 'src').mkdir()
        (self.root / 'src/main.c').write_text('int x;\n')
        (self.root / 'dump.rom').write_bytes(b'private placeholder')
        (self.root / '.env').write_text('SECRET=placeholder\n')
        real_git = self.backup.git
        def offline(*args,**kwargs):
            if args[0]=='push': raise module.SyncError('simulated network failure')
            return real_git(*args,**kwargs)
        with patch.object(self.backup,'git',side_effect=offline):
            with self.assertRaisesRegex(module.SyncError,'network failure'): self.backup.cycle()
            head=self.git('rev-parse','HEAD')
            with self.assertRaisesRegex(module.SyncError,'network failure'): self.backup.cycle()
            self.assertEqual(head,self.git('rev-parse','HEAD'))
        names=self.git('ls-tree','-r','--name-only','HEAD').splitlines()
        self.assertIn('src/main.c',names)
        self.assertNotIn('dump.rom',names)
        self.assertNotIn('.env',names)
        self.assertIn('[skip ci]',self.git('log','-1','--format=%s'))

    def test_manual_staging_stops_backup(self):
        (self.root / 'README.md').write_text('manual\n'); self.git('add','README.md')
        with self.assertRaisesRegex(module.SyncError,'staged'): self.backup.cycle()

    def test_wrong_branch_and_remote_rejected(self):
        self.git('switch','-c','main')
        with self.assertRaisesRegex(module.SyncError,'branch'): self.backup.cycle()
        self.git('switch','dev')
        self.config['remote_url']='https://github.com/someone/other.git'
        with self.assertRaisesRegex(module.SyncError,'Remote differs'): self.backup.cycle()

    def test_non_fast_forward_preserves_other_history(self):
        self.require_push()
        other = self.base / 'other'
        subprocess.run(['git','clone','--branch','dev',str(self.origin),str(other)],check=True,capture_output=True)
        subprocess.run(['git','-C',str(other),'config','user.name','Other test'],check=True)
        subprocess.run(['git','-C',str(other),'config','user.email','other@example.invalid'],check=True)
        (other / 'README.md').write_text('remote work\n')
        for args in [('add','README.md'),('commit','-m','other change'),('push','origin','dev')]:
            subprocess.run(['git','-C',str(other),*args],check=True,capture_output=True)
        remote_head=subprocess.check_output(['git','--git-dir',str(self.origin),'rev-parse','dev'],text=True).strip()
        (self.root / 'README.md').write_text('local work\n')
        with self.assertRaises(module.SyncError): self.backup.cycle()
        self.assertEqual(remote_head,subprocess.check_output(['git','--git-dir',str(self.origin),'rev-parse','dev'],text=True).strip())
        self.assertEqual('local work\n',(self.root / 'README.md').read_text())

    def test_recent_saves_wait_and_session_lock(self):
        self.config['quiet_seconds']=60
        (self.root / 'README.md').write_text('still editing\n')
        self.assertIn('settle',self.backup.cycle())
        lock = module.lock_session(self.backup)
        try:
            with self.assertRaises(module.SyncError): module.lock_session(self.backup)
        finally: lock.close()
        again = module.lock_session(self.backup); again.close()

if __name__ == '__main__': unittest.main()
