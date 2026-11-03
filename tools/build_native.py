"""CEdev v15 native build stages, without GNU Make. No shell or deletions."""
import argparse, subprocess
from pathlib import Path
r=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--tests',action='store_true');args=a.parse_args()
t=r/'.tools/CEdev';o=r/'obj'/('tests-native' if args.tests else 'native');o.mkdir(parents=True,exist_ok=True)
b=r/'bin';b.mkdir(exist_ok=True);name='TESTCE' if args.tests else 'LAYERCE'
def run(exe,*flags):
 print('[build]',Path(exe).name,flush=True)
 subprocess.run([str(exe),*[str(f).replace(chr(92), chr(47)) for f in flags]],cwd=r,check=True)
cc=t/'bin/ez80-clang.exe'; assembler=t/'binutils/bin/z80-none-elf-as.exe';ld=t/'binutils/bin/z80-none-elf-ld.exe'
flags=['-nostdinc','-Xclang','-fforce-mangle-main-argc-argv','-fno-autolink','-fno-addrsig','-fno-threadsafe-statics','-fno-aligned-allocation','-mllvm','-profile-guided-section-prefix=false','-mllvm','-z80-gas-style','-ffunction-sections','-fdata-sections','-fno-math-errno','-D__TICE__=1','-isystem',str(t/'include'),'-Isrc','-Wall','-Wextra','-Oz','-std=c11','-DNDEBUG=1']
if args.tests: flags+=['-DCORE_TEST']
sources=['src/game.c','tests/core_tests.c'] if args.tests else ['src/game.c','src/main.c']
objects=[]
for source in sources:
 stem=Path(source).stem;s=o/(stem+'.s');obj=o/(stem+'.o')
 run(cc,'-S',*flags,source,'-o',s);run(assembler,'-march=ez80+full',s,'-o',obj);objects.append(obj)
libs=[t/'lib/crt/libcrt.a',t/'lib/ce/libce.a',t/'lib/softfloat/libsoftfloat.a',t/'lib/libc/libc_os.a',t/'lib/libcxx/libcxx.a']
common=['-static','--entry=__start','--gc-sections','--omagic','--defsym','__TICE__=1']
partial=o/(name+'.o')
run(ld,'-i',*common,'--build-id=none','--defsym','NDEBUG=1',*objects,'--whole-archive',t/'lib/libc/libnanoprintf.a','--no-whole-archive','--start-group',*libs,'--end-group','-o',partial)
run(t/'binutils/bin/z80-none-elf-strip.exe','--strip-unneeded',partial)
run(t/'bin/cedev-obj.exe','--elf',partial,'--output',o/'crt.h','--libs',*[p for p in (t/'lib/libload').glob('*.lib') if p.name!='libload.lib'])
run(cc,'-I'+str(o),'-E','-P',t/'lib/crt/crt0.S','-o',o/'crt0.s')
run(assembler,'-I'+str(o),'-march=ez80+full',o/'crt0.s','-o',o/'crt0.o')
run(ld,*common,'--no-warn-rwx-segments','--defsym=LOAD_ADDR=0xD1A87F','--defsym=BSSHEAP_LOW=0xD052C6','--defsym=BSSHEAP_HIGH=0xD13FD8','--defsym=STACK_HIGH=0xD1A87E','-T'+str(t/'meta/linker_script.ld'),'-Map='+str(b/(name+'.map')),partial,o/'crt0.o','--start-group',t/'lib/libc/allocator_standard.a',*libs,'--end-group','-o',b/(name+'.obj'))
run(t/'bin/convbin.exe','-r','-k','8xp','-u','-n',name,'-j','elf','-i',b/(name+'.obj'),'-o',b/(name+'.8xp'))
print('BUILT',b/(name+'.8xp'),(b/(name+'.8xp')).stat().st_size,'bytes')
import hashlib, json, re
maptext=(b/(name+'.map')).read_text()
measurements={}
for symbol in ['___bss_len','___heap_low','___heap_high']:
 match=re.search(r'(0x[0-9a-fA-F]+)\s+'+symbol+r'\b',maptext)
 if match: measurements[symbol]=int(match.group(1),16)
measurements['file_bytes']=(b/(name+'.8xp')).stat().st_size
measurements['sha256']=hashlib.sha256((b/(name+'.8xp')).read_bytes()).hexdigest()
(b/(name+'-build.json')).write_text(json.dumps(measurements,indent=2))
print(json.dumps(measurements,indent=2))
