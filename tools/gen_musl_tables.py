#!/usr/bin/env python3
# Emit the Nelua table declarations in src/math_core.nelua from musl 1.2.5
# (MIT) src/math/{exp_data,pow_data,log_data,log2_data}.c.
import re, sys, struct
# usage: tools/gen_musl_tables.py path/to/musl-1.2.5/src/math > tables.nelua
d = sys.argv[1]
def body(path, start, end=None):
    s = open(path).read()
    i = s.index(start)
    j = s.index(end, i) if end else len(s)
    return s[i+len(start):j]
def strip(s):
    s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    s = re.sub(r'//.*', '', s)
    return s
def emit_u64(name, toks):
    print(f"local {name}: [{len(toks)}]culonglong = {{")
    for k in range(0, len(toks), 2):
        print("  " + ", ".join(t + "_culonglong" for t in toks[k:k+2]) + ",")
    print("}")
def emit_dbl(name, toks):
    print(f"local {name}: [{len(toks)}]cdouble = {{")
    for k in range(0, len(toks), 2):
        print("  " + ", ".join(t.upper().replace('0X','0x').replace('P','P') + "_cdouble" for t in toks[k:k+2]) + ",")
    print("}")
def hexf(t):  # normalize hex float literal for Nelua: 0x1.abcP-3
    t = t.strip()
    if not t.lstrip('-').lower().startswith('0x'): raise SystemExit("not hex: "+t)
    return t
# exp_data
s = strip(body(d+"/exp_data.c", ".tab = {", "},"))
toks = [t.strip() for t in s.replace("\n"," ").split(",") if t.strip()]
assert len(toks) == 256, len(toks)
emit_u64("exp_data_tab", toks)
# pow_data: struct {invc, pad, logc, logctail}
s = strip(body(d+"/pow_data.c", ".tab = {", "\n},"))
rows = [r for r in re.findall(r'A\(([^)]*)\)', s) if 'x' in r.split(',')[0]]
assert len(rows) == 128, len(rows)
invc=[];logc=[];logctail=[]
for r in rows:
    f = [t.strip() for t in r.split(",") if t.strip()]
    assert len(f) == 3
    invc.append(hexf(f[0])); logc.append(hexf(f[1])); logctail.append(hexf(f[2]))
emit_dbl("pow_log_invc", invc); emit_dbl("pow_log_logc", logc); emit_dbl("pow_log_logctail", logctail)
# log_data: tab {invc, logc} 128, tab2 {chi, clo} 128
s = strip(body(d+"/log_data.c", ".tab = {", "\n},"))
rows = re.findall(r'\{([^}]*)\}', s); assert len(rows)==128, len(rows)
emit_dbl("log_tab_invc", [hexf(r.split(",")[0]) for r in rows]); emit_dbl("log_tab_logc", [hexf(r.split(",")[1]) for r in rows])
s = strip(body(d+"/log_data.c", ".tab2 = {", "\n},"))
rows = re.findall(r'\{([^}]*)\}', s); assert len(rows)==128, len(rows)
emit_dbl("log_tab2_chi", [hexf(r.split(",")[0]) for r in rows]); emit_dbl("log_tab2_clo", [hexf(r.split(",")[1]) for r in rows])
# log2_data: tab {invc, logc} 64, tab2 {chi, clo} 64
s = strip(body(d+"/log2_data.c", ".tab = {", "\n},"))
rows = re.findall(r'\{([^}]*)\}', s); assert len(rows)==64, len(rows)
emit_dbl("log2_tab_invc", [hexf(r.split(",")[0]) for r in rows]); emit_dbl("log2_tab_logc", [hexf(r.split(",")[1]) for r in rows])
s = strip(body(d+"/log2_data.c", ".tab2 = {", "\n},"))
rows = re.findall(r'\{([^}]*)\}', s); assert len(rows)==64, len(rows)
emit_dbl("log2_tab2_chi", [hexf(r.split(",")[0]) for r in rows]); emit_dbl("log2_tab2_clo", [hexf(r.split(",")[1]) for r in rows])
