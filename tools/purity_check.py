#!/usr/bin/env python3
"""Check that query contracts optimize calls without erasing observable effects."""
import argparse,re,shlex,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--cc',default='gcc');p.add_argument('--out',type=Path,default=Path('build/purity-contract.s'))
a=p.parse_args();root=Path(__file__).resolve().parents[1];a.out.parent.mkdir(parents=True,exist_ok=True)
subprocess.run(shlex.split(a.cc)+['-O2','-fno-builtin','-frounding-math','-I'+str(root/'include'),'-S',str(root/'tools/purity_contract.c'),'-o',str(a.out)],check=True)
assembly=a.out.read_text()
expected={'repeated_query':('strcmp',1),'query_after_store':('strcmp',2),'discard_query':('strcmp',0),'repeated_scalar':('isalpha',1),'ctype_after_locale':('isalpha',2),'retain_write':('strcpy',1),'retain_conversion':('strtol',1),'retain_math':('sin',1)}
for function,(callee,count) in expected.items():
 match=re.search(r'^'+function+r':[^\n]*\n(.*?)^\s*\.size\s+'+function+r'\b',assembly,re.M|re.S)
 if not match:raise RuntimeError('Missing assembly for '+function)
 actual=len(re.findall(r'\b(?:callq?|jmpq?)\s+'+callee+r'(?:@PLT)?\b',match.group(1)))
 if actual!=count:raise RuntimeError(f'{function}: expected {count} {callee} calls, observed {actual}')
print(a.cc+': repeated/unused queries optimize; memory writes, locale changes, output parameters and fenv calls remain observable')
