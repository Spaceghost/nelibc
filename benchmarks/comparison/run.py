#!/usr/bin/env python3
"""Matched production-archive comparison; retain commands, maps, raw runs and failures."""
import argparse, csv, hashlib, io, json, math, os, platform, re, shlex, statistics, subprocess, time
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
def command(args, **kwargs):
    try:
        return subprocess.run([str(x) for x in args], check=True, text=True, **kwargs)
    except subprocess.CalledProcessError as error:
        if error.stderr: print(error.stderr, flush=True)
        raise
def profiles(cosmo, zig):
    makefile=(ROOT/'Makefile').read_text().replace('\\\n',' ')
    wraps=shlex.split(re.search(r'^MUSL_STDIO_WRAP_FLAGS \?= (.*)$',makefile,re.M).group(1))
    return {
      'glibc': {'cc':['gcc'], 'flags':[], 'archive':'build/libnelibc-gcc.a','link':[], 'version':command(['getconf','GNU_LIBC_VERSION'],capture_output=True).stdout.strip()},
      'musl': {'cc':[zig,'cc','-mcpu=x86_64','-target','x86_64-linux-musl'], 'flags':['-static'], 'archive':'build/libnelibc-zig-musl.a','link':wraps, 'version':'Zig 0.17.0 bundled musl; see source/toolchain manifest'},
      'cosmopolitan': {'cc':[cosmo], 'flags':[], 'archive':'build/libnelibc-cosmo.a','link':[], 'version':'4.0.2'},
    }
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build-only',action='store_true')
    p.add_argument('--previous-dir',type=Path,help='Saved production archives for a concurrent before/after comparison')
    p.add_argument('--out',required=True,type=Path);p.add_argument('--cosmocc',required=True);p.add_argument('--zig',default='/opt/zig/zig');p.add_argument('--pairs',type=int,default=7);p.add_argument('--cpu',type=int)
    args=p.parse_args();out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    cfg=profiles(args.cosmocc,args.zig)
    implementations=['baseline','nelibc'] if not args.previous_dir else ['baseline','previous','nelibc']
    meta={'date_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'platform':platform.platform(),'cpu_model':re.search(r'model name\s*:\s*(.*)',Path('/proc/cpuinfo').read_text()).group(1),'allowed_cpus':sorted(os.sched_getaffinity(0)),'cpu':args.cpu,'load_start':os.getloadavg(),'pairs':args.pairs,'profiles':cfg,'commands':[],'source_base':command(['git','rev-parse','HEAD'],cwd=ROOT,capture_output=True).stdout.strip(),'source_diff_sha256':hashlib.sha256(command(['git','diff'],cwd=ROOT,capture_output=True).stdout.encode()).hexdigest(),'benchmark_sha256':hashlib.sha256((ROOT/'benchmarks/comparison/consumer.c').read_bytes()).hexdigest(),'compiler_versions':{}}
    tracked=command(['git','ls-files','-z'],cwd=ROOT,capture_output=True).stdout.split('\0')
    untracked=command(['git','ls-files','--others','--exclude-standard','-z'],cwd=ROOT,capture_output=True).stdout.split('\0')
    meta['source_files']={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in sorted(set(tracked+untracked)) if name and (ROOT/name).is_file() and '__pycache__' not in name}
    meta['archive_sha256']={name:hashlib.sha256((ROOT/c['archive']).read_bytes()).hexdigest() for name,c in cfg.items()}
    if args.previous_dir:
      meta['previous_archive_sha256']={name:hashlib.sha256((args.previous_dir/Path(c['archive']).name).read_bytes()).hexdigest() for name,c in cfg.items()}
    checks={}; binaries={}; ownership={};coverage={}
    inventory=list(csv.DictReader((ROOT/'docs/musl-1.2.5-api-coverage.csv').open()))
    flags=['-O2','-fno-builtin','-fno-stack-protector','-fno-sanitize=all']
    for profile,c in cfg.items():
      meta['compiler_versions'][profile]=command(c['cc']+['--version'],capture_output=True).stdout.splitlines()[0]
      symbols=set()
      for line in command(['nm','-g','--defined-only',ROOT/c['archive']],capture_output=True).stdout.splitlines():
        fields=line.split()
        if len(fields)>=3: symbols.add(fields[-1])
      ownership[profile]=sorted(symbols)
      coverage[profile]={'inventory_total':len(inventory),'nelibc_exports':sum(r['api'] in symbols for r in inventory),'exports':[r['api'] for r in inventory if r['api'] in symbols]}
      binaries[profile]={}
      for implementation in implementations:
        print(f'build/check: {profile}-{implementation}',flush=True)
        binary=out/f'{profile}-{implementation}'
        cmd=c['cc']+flags+c['flags']+[str(ROOT/'benchmarks/comparison/consumer.c')]
        if implementation!='baseline':
          archive=ROOT/c['archive'] if implementation=='nelibc' else args.previous_dir.resolve()/Path(c['archive']).name
          cmd+=c['link']+[str(archive)]
        if profile!='musl':cmd += ['-Wl,-Map,'+str(binary)+'.map']
        cmd += ['-lm','-pthread','-o',str(binary)]
        meta['commands'].append(cmd);command(cmd,cwd=ROOT,capture_output=True)
        binaries[profile][implementation]=str(binary)
        (out/f'{profile}-{implementation}.symbols').write_text(command(['nm','-g','--defined-only',binary],capture_output=True).stdout)
        result=subprocess.run([binary,'--check'],text=True,capture_output=True,timeout=30)
        (out/f'{profile}-{implementation}.checks.log').write_text(result.stdout+result.stderr)
        checks[profile+'-'+implementation]={'exit':result.returncode,'result':json.loads(result.stdout) if result.stdout.strip().startswith('{') else result.stdout,'stderr':result.stderr}
        if result.returncode:raise RuntimeError(f'consumer checks fail: {profile} {implementation}: {result.stderr}')
    (out/'manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
    (out/'checks.json').write_text(json.dumps(checks,indent=2)+'\n')
    (out/'coverage.json').write_text(json.dumps(coverage,indent=2)+'\n')
    if args.build_only:return
    if args.cpu is not None:os.sched_setaffinity(0,{args.cpu})
    raw=[]
    for pair in range(1,args.pairs+1):
      # Rotate profile order as well as alternating the two implementations.
      names=list(cfg); names=names[(pair-1)%len(names):]+names[:(pair-1)%len(names)]
      for profile in names:
        if len(implementations)==2:
          order=implementations if pair%2 else implementations[::-1]
        else:
          order=implementations[(pair-1)%len(implementations):]+implementations[:(pair-1)%len(implementations)]
          if pair%2==0:order=order[::-1]
        for implementation in order:
          result=command([binaries[profile][implementation]],capture_output=True,timeout=60)
          (out/f'{profile}-{implementation}-{pair}.csv').write_text(result.stdout)
          for row in csv.DictReader(io.StringIO(result.stdout)):
            raw.append({'profile':profile,'implementation':implementation,'pair':pair,'order':'/'.join(order),**row})
        print(f'pair {pair}/{args.pairs}: {profile}',flush=True)
    fields=list(raw[0]);
    with (out/'raw.csv').open('w') as f:
      w=csv.DictWriter(f,fields);w.writeheader();w.writerows(raw)
    grouped={}
    symbol_for={'memmove-overlap':'memmove','memcmp-equal':'memcmp','memchr-absent':'memchr','strcmp-equal':'strcmp','strstr-tail':'strstr','strtol-decimal':'strtol','strtod-decimal':'strtod','malloc-free':'malloc','snprintf-mixed':'snprintf','qsort-128-integers':'qsort'}
    for r in raw:grouped.setdefault((r['profile'],r['case'],r['bytes']),{}).setdefault(r['implementation'],{})[int(r['pair'])]=float(r['ns_per_op'])
    summary=[]
    for (profile,case,size),r in grouped.items():
      ratios=[r['baseline'][i]/r['nelibc'][i] for i in r['baseline']]
      symbol=symbol_for.get(case,case)
      row={'profile':profile,'case':case,'family':next(x['family'] for x in raw if x['case']==case),'bytes':int(size),'ownership':'mixed workload' if case=='parse-record' else ('Nelibc export' if symbol in ownership[profile] else 'delegated'),'baseline_ns':statistics.median(r['baseline'].values()),'nelibc_ns':statistics.median(r['nelibc'].values()),'speedup':statistics.median(ratios),'ratio_min':min(ratios),'ratio_max':max(ratios),'pairs':len(ratios)}
      if 'previous' in r:
        previous_ratios=[r['previous'][i]/r['nelibc'][i] for i in r['nelibc']]
        row.update(previous_ns=statistics.median(r['previous'].values()),speedup_vs_previous=statistics.median(previous_ratios),previous_ratio_min=min(previous_ratios),previous_ratio_max=max(previous_ratios))
      summary.append(row)
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    with (out/'summary.csv').open('w') as f:
      w=csv.DictWriter(f,list(summary[0]));w.writeheader();w.writerows(summary)
    meta['load_end']=os.getloadavg();meta['binary_bytes']={p:{i:Path(b).stat().st_size for i,b in bs.items()}for p,bs in binaries.items()}
    (out/'manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
if __name__=='__main__':main()
