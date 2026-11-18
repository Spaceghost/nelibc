#!/usr/bin/env python3
"""Matched GCC/glibc strcmp timing across sizes, alignments and mismatch positions."""
import argparse, csv, hashlib, io, json, os, shlex, statistics, subprocess
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--out', required=True, type=Path)
p.add_argument('--old-archive', type=Path)
p.add_argument('--pairs', type=int, default=7)
p.add_argument('--cpu', type=int)
p.add_argument('--source',type=Path,help='Alternate consumer with the same CSV columns, e.g. tools/memory_bench.c')
a = p.parse_args()
root = Path(__file__).resolve().parents[1]
source=a.source.resolve() if a.source else root/'tools/strcmp_bench.c'
out = a.out.resolve()
out.mkdir(parents=True, exist_ok=True)
archives = {'baseline': None, 'candidate': root/'build/libnelibc-gcc.a'}
if a.old_archive:
    archives['previous'] = a.old_archive.resolve()
commands = {}
for name, archive in archives.items():
    cmd = ['gcc', '-O2', '-fno-builtin', '-fno-stack-protector',
           str(source)]
    if archive:
        cmd.append(str(archive))
    cmd += ['-lm', '-pthread', '-o', str(out/name)]
    commands[name] = cmd
    subprocess.run(cmd, check=True)
meta = {'commands': commands, 'pairs': a.pairs, 'cpu': a.cpu,
        'load_start': os.getloadavg(),
        'archive_sha256': {n: hashlib.sha256(v.read_bytes()).hexdigest()
                           for n, v in archives.items() if v},
        'source':str(source), 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest()}
if a.cpu is not None:
    os.sched_setaffinity(0, {a.cpu})
raw = []
for pair in range(1, a.pairs+1):
    names = list(archives)
    offset = (pair-1) % len(names)
    names = names[offset:] + names[:offset]
    if pair % 2 == 0:
        names.reverse()
    for name in names:
        result = subprocess.run([str(out/name)], check=True, text=True, capture_output=True)
        (out/f'{name}-{pair}.csv').write_text(result.stdout)
        raw.extend({'implementation': name, 'pair': pair, **r}
                   for r in csv.DictReader(io.StringIO(result.stdout)))
    print(f'strcmp distribution pair {pair}/{a.pairs}', flush=True)
with (out/'raw.csv').open('w') as f:
    w = csv.DictWriter(f, list(raw[0]))
    w.writeheader()
    w.writerows(raw)
groups = {}
for r in raw:
    key = (int(r['bytes']), int(r['left_offset']), int(r['right_offset']), r['case'])
    groups.setdefault(key, {}).setdefault(r['implementation'], {})[r['pair']] = float(r['ns_per_op'])
summary = []
for (size, left, right, case), values in groups.items():
    ratios = [values['baseline'][i]/v for i, v in values['candidate'].items()]
    row = {'bytes': size, 'left_offset': left, 'right_offset': right, 'case': case,
           'median_ns': {n: statistics.median(v.values()) for n, v in values.items()},
           'candidate_speedup_vs_glibc': statistics.median(ratios),
           'ratio_min': min(ratios), 'ratio_max': max(ratios)}
    if 'previous' in values:
        row['candidate_speedup_vs_previous'] = statistics.median(
            values['previous'][i]/v for i, v in values['candidate'].items())
    summary.append(row)
(out/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
meta['load_end'] = os.getloadavg()
(out/'manifest.json').write_text(json.dumps(meta, indent=2)+'\n')
