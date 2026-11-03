"""VS Code session backup of saved project files. Python stdlib + Git only.
Never pulls, merges, resets, force-pushes, or modifies game source.
"""
import argparse
import datetime
import json
import os
from pathlib import Path, PurePosixPath
import subprocess
import sys
import time

ROOT_FILES = {'makefile', 'README.md', 'LICENSE', '.gitignore', '.gitattributes',
              'LayerDefenseCE.code-workspace'}
EXTENSIONS = {'src': {'.c', '.h'}, 'tests': {'.c', '.h', '.py'},
              'docs': {'.md'}, 'tools': {'.py', '.ps1', '.json'},
              '.vscode': {'.json'}, '.github': {'.yml', '.yaml'}}

def permitted(name):
    path = PurePosixPath(name)
    if path.is_absolute() or '..' in path.parts or not path.parts:
        return False
    if name in ROOT_FILES:
        return True
    if path.parts[0] not in EXTENSIONS or len(path.parts) < 2:
        return False
    if any(p.startswith('.') for p in path.parts[1:]):
        return False
    return path.suffix.lower() in EXTENSIONS[path.parts[0]]

class SyncError(RuntimeError):
    pass

class Backup:
    def __init__(self, root, config):
        self.root = Path(root).resolve()
        self.config = config
        self.env = dict(os.environ, GIT_TERMINAL_PROMPT='0', GCM_INTERACTIVE='never')

    def git(self, *args, check=True):
        p = subprocess.run(['git', *args], cwd=self.root, env=self.env,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           encoding='utf-8', errors='replace', timeout=45)
        if check and p.returncode:
            raise SyncError((p.stderr or p.stdout).strip() or 'Git operation failed')
        return p

    def validate(self):
        root = Path(self.git('rev-parse', '--show-toplevel').stdout.strip()).resolve()
        if root != self.root:
            raise SyncError('Run only from the game repository root')
        branch = self.git('branch', '--show-current').stdout.strip()
        if branch != self.config['branch']:
            raise SyncError('Auto backup paused: switch to branch ' + self.config['branch'])
        url = self.git('remote', 'get-url', self.config['remote']).stdout.strip()
        if url.lower().rstrip('/') != self.config['remote_url'].lower().rstrip('/'):
            raise SyncError('Remote differs from tools/sync-config.json; inspect it manually')
        git_dir = Path(self.git('rev-parse', '--absolute-git-dir').stdout.strip())
        if any((git_dir / p).exists() for p in
               ('index.lock', 'MERGE_HEAD', 'CHERRY_PICK_HEAD', 'rebase-merge', 'rebase-apply')):
            raise SyncError('Git operation in progress; finish it before automatic backup')
        if self.git('diff', '--cached', '--quiet', check=False).returncode:
            raise SyncError('Manually staged files found; commit or unstage them first')

    def candidates(self):
        changed = self.git('diff', '--name-only', '--no-renames', '-z').stdout
        untracked = self.git('ls-files', '--others', '--exclude-standard', '-z').stdout
        return sorted({p for p in (changed + untracked).split('\0') if p and permitted(p)})

    def cycle(self, dry_run=False):
        self.validate()
        paths = self.candidates()
        if dry_run:
            return 'Dry run; would back up: ' + (', '.join(paths) or '(no saved changes)')
        if paths:
            quiet = self.config.get('quiet_seconds', 10)
            if any((self.root / p).exists() and
                   time.time()-(self.root / p).stat().st_mtime < quiet for p in paths):
                return 'Waiting for saved files to settle'
            # --only prevents unrelated concurrent staged files entering our commit.
            self.git('add', '-A', '--', *paths)
            stamp = datetime.datetime.now().astimezone().isoformat(timespec='seconds')
            self.git('commit', '--only', '-m', 'WIP backup ' + stamp + ' [skip ci]', '--', *paths)
        branch, remote = self.config['branch'], self.config['remote']
        tracking = self.git('rev-parse', '--verify', 'refs/remotes/' + remote + '/' + branch,
                            check=False)
        head = self.git('rev-parse', 'HEAD').stdout.strip()
        if tracking.returncode == 0 and tracking.stdout.strip() == head:
            return 'Up to date; no new saved changes'
        # No force, no automatic pull. A divergent branch will be rejected by Git.
        self.git('push', remote, 'HEAD:refs/heads/' + branch)
        return 'Backed up to ' + remote + '/' + branch + ' (' + head[:8] + ')'

def log(message):
    print(time.strftime('%H:%M:%S') + ' ' + message, flush=True)

def lock_session(backup):
    git_dir = Path(backup.git('rev-parse', '--absolute-git-dir').stdout.strip())
    handle = (git_dir / 'layerce-session.lock').open('a+b')
    handle.seek(0, os.SEEK_END)
    if handle.tell() == 0:
        handle.write(b'0'); handle.flush()
    handle.seek(0)
    try:
        if os.name == 'nt':
            import msvcrt
            msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
        else:
            import fcntl
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.close()
        raise SyncError('Another automatic backup task already owns this workspace')
    return handle  # OS releases lock even if VS Code terminates the process.

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--once', action='store_true')
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    config = json.loads((root / 'tools/sync-config.json').read_text(encoding='utf-8'))
    backup = Backup(root, config)
    session = None
    try:
        session = lock_session(backup)
        log('SYNC_READY: saved project files only; builds remain manual')
        while True:
            try:
                log(backup.cycle(args.dry_run))
            except (SyncError, subprocess.TimeoutExpired, OSError) as error:
                log('SYNC_ERROR: ' + str(error))
                log('Local files retained. Check Git login/network/branch in VS Code.')
                if args.once or args.dry_run:
                    return 1
            if args.once or args.dry_run:
                return 0
            time.sleep(max(10, config.get('interval_seconds', 120)))
    except (SyncError, OSError) as error:
        log('SYNC_ERROR: ' + str(error))
        return 1
    except KeyboardInterrupt:
        log('Automatic backup stopped')
        return 0
    finally:
        if session:
            session.close()

if __name__ == '__main__':
    sys.exit(main())
