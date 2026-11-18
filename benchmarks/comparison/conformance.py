#!/usr/bin/env python3
"""Run the same pinned musl libc-test corpus with and without each Nelibc archive.

Uses each libc's own headers for BOTH halves of a pair. This isolates library
replacement from Nelibc header coverage. Functional, regression and math tests
are measured; the compile-only API catalogue is reported separately by symbols.
No expected-failure list suppresses results. The native libc-test supervisor
retains its 100 KiB stack limit and five-second per-test timeout.
"""
import argparse, csv, io, json, os, re, shlex, subprocess, tarfile
from pathlib import Path
from run import ROOT, command, profiles

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--resume',action='store_true');p.add_argument('--profile',choices=['glibc','musl','cosmopolitan']);p.add_argument('--ape-loader',type=Path)
    p.add_argument('--source',required=True,type=Path);p.add_argument('--out',required=True,type=Path);p.add_argument('--cosmocc',required=True);p.add_argument('--zig',default='/opt/zig/zig')
    args=p.parse_args();out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    commit=command(['git','rev-parse','HEAD'],cwd=args.source,capture_output=True).stdout.strip()
    source=subprocess.check_output(['git','-C',str(args.source),'archive',commit])
    def extract(dest):
      dest.mkdir(parents=True,exist_ok=False)
      subprocess.run(['tar','-xf','-','-C',str(dest)],input=source,check=True)
      make=dest/'Makefile';s=make.read_text();s=s.replace('bin.exe bin-static.exe','bin.exe');make.write_text(s)
    host=out/'supervisor'
    if not args.resume:extract(host)
    command(['make','-j2','src/common/runtest.exe'],cwd=host,stdout=(host/'build.log').open('w'),stderr=subprocess.STDOUT)
    cfg=profiles(args.cosmocc,args.zig);summaries={};all_tests={}
    if args.resume:
      old=json.loads((out/'results.json').read_text());summaries=old['profiles'];all_tests=old['tests']
      (out/'results-initial-launch.json').write_text(json.dumps(old,indent=2)+'\n')
    if args.profile:cfg={args.profile:cfg[args.profile]}
    if 'cosmopolitan' in cfg and not args.ape_loader:raise RuntimeError('Cosmopolitan .exe tests require --ape-loader (the native loader extracted by cosmocc, e.g. $TMPDIR/.ape-1.10)')
    for profile,c in cfg.items():
      for implementation in ['baseline','nelibc']:
        name=profile+'-'+implementation;dest=out/name
        if not args.resume:extract(dest)
        link=[]
        if implementation=='nelibc':link=c['link']+[str(ROOT/c['archive'])]
        libs=['-lpthread','-lm','-lrt']
        if profile=='glibc':libs+=['-lcrypt','-ldl','-lresolv','-lutil']
        flags=['-O2','-pipe','-std=c99','-D_POSIX_C_SOURCE=200809L','-fno-builtin','-frounding-math','-Wno-unknown-pragmas','-Werror=implicit-function-declaration','-Werror=implicit-int','-Werror=pointer-sign','-Werror=pointer-arith']
        ldflags=c['flags']+(c['link'] if implementation=='nelibc' else [])
        config='CC := '+shlex.join(c['cc'])+'\nCFLAGS += '+shlex.join(flags)+'\nLDFLAGS += -s '+shlex.join(ldflags)+'\nLDLIBS += '+shlex.join(libs)+'\nRUN_TEST := '+str(host/'src/common/runtest.exe')+(' -w '+str(args.ape_loader.resolve()) if profile=='cosmopolitan' else '')+'\n'
        if implementation=='nelibc':config+='LDLIBS := '+str(ROOT/c['archive'])+' $(LDLIBS)\n'
        (dest/'config.mak').write_text(config)
        debug=command(['make','debug'],cwd=dest,capture_output=True).stdout
        binaries=re.search(r'^BINS (.*)$',debug,re.M).group(1).split()
        planned=[x for x in binaries if re.match(r'src/(functional|regression|math)/.*\.exe$',x)]
        make=['make','-k','-j2']
        if args.resume:
          (dest/'build-initial-launch.log').write_bytes((dest/'build.log').read_bytes())
          for name2 in planned:
            if (dest/name2).exists():make+=['-W',name2]
        with (dest/'build.log').open('w') as log:
          result=subprocess.run(make+['src/functional/REPORT','src/regression/REPORT','src/math/REPORT'],cwd=dest,stdout=log,stderr=subprocess.STDOUT)
        tests={}
        for binary in planned:
          path=dest/binary;key=binary.removeprefix('src/').removesuffix('.exe');err=path.with_suffix('.err')
          msg=err.read_text(errors='replace') if err.exists() else ''
          if not path.exists():
            status='link_failure' if path.with_suffix('.o').exists() else 'build_failure'
            diagnostic=Path(str(path)+'.ld.err') if status=='link_failure' else path.with_suffix('.o.err')
            if diagnostic.exists():msg=diagnostic.read_text(errors='replace')
          elif not err.exists():status='not_run'
          elif 'FAIL ' in msg:status='timeout' if 'timed out' in msg else 'runtime_failure'
          else:status='pass'
          tests[key]={'status':status,'output':msg[:3000]}
        counts={s:sum(t['status']==s for t in tests.values()) for s in ['pass','build_failure','link_failure','runtime_failure','timeout','not_run']}
        summaries[name]={'planned':len(tests),'counts':counts,'make_exit':result.returncode}
        all_tests[name]=tests
        (out/'results.json').write_text(json.dumps({'libc_test_commit':commit,'method':'libc-owned headers, functional/regression/math; native libc-test supervisor, 100 KiB stack, 5 s timeout; Cosmopolitan .exe files via official APE loader','profiles':summaries,'tests':all_tests},indent=2)+'\n')
        print(name,json.dumps(summaries[name]),flush=True)
    deltas={}
    for profile in ['glibc','musl','cosmopolitan']:
      if profile+'-nelibc' not in all_tests:continue
      base=all_tests[profile+'-baseline'];nel=all_tests[profile+'-nelibc']
      deltas[profile]={'regressions':[k for k,v in nel.items() if v['status']!='pass' and base[k]['status']=='pass'],'improvements':[k for k,v in nel.items() if v['status']=='pass' and base[k]['status']!='pass']}
    (out/'deltas.json').write_text(json.dumps(deltas,indent=2)+'\n')
if __name__=='__main__':main()
