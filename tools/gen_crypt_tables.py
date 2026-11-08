#!/usr/bin/env python3
# Emit the Nelua table declarations in src/crypt_standalone.nelua from musl
# 1.2.5 (MIT) src/crypt/crypt_des.c, crypt_md5.c, crypt_sha256.c,
# crypt_sha512.c and crypt_blowfish.c.
# usage: tools/gen_crypt_tables.py path/to/musl-1.2.5/src/crypt > tables.nelua
import re, sys
d = sys.argv[1]

def strip(s):
    return re.sub(r'/\*.*?\*/', '', s, flags=re.S)

def array_body(src, decl):
    i = src.index(decl)
    i = src.index('{', i)
    depth = 0
    for j in range(i, len(src)):
        if src[j] == '{': depth += 1
        elif src[j] == '}':
            depth -= 1
            if depth == 0: return src[i:j+1]
    raise SystemExit('unterminated ' + decl)

def words(body):
    return [int(t, 16) for t in re.findall(r'0x[0-9a-fA-F]+', body)]

def emit(name, dims, vals, ty='uint32', fmt='0x%08x', per=8):
    n = 1
    for x in dims: n *= x
    assert len(vals) == n, (name, len(vals), n)
    t = ''.join('[%d]' % x for x in dims)
    print('local %s: %s%s = {' % (name, t, ty))
    if len(dims) == 1:
        rows = [vals]
    else:
        inner = n // dims[0]
        rows = [vals[k*inner:(k+1)*inner] for k in range(dims[0])]
    for r in rows:
        if len(dims) > 1: print('  {')
        for k in range(0, len(r), per):
            print('    ' + ', '.join(fmt % v for v in r[k:k+per]) + ',')
        if len(dims) > 1: print('  },')
    print('}')

des = strip(open(d + '/crypt_des.c').read())
for name, dims in [('psbox', (8, 64)), ('ip_maskl', (16, 16)), ('ip_maskr', (16, 16)),
                   ('fp_maskl', (8, 16)), ('fp_maskr', (8, 16)),
                   ('key_perm_maskl', (8, 16)), ('key_perm_maskr', (12, 16)),
                   ('comp_maskl0', (4, 8)), ('comp_maskr0', (4, 8)),
                   ('comp_maskl1', (4, 16)), ('comp_maskr1', (4, 16))]:
    emit('des_' + name, dims, words(array_body(des, 'uint32_t %s[' % name)))

bf = strip(open(d + '/crypt_blowfish.c').read())
emit('bf_magic_w', (6,), words(array_body(bf, 'BF_magic_w[6]')))
# P (18 words) then S[0..3] (256 each): the layout of BF_ctx.PS.
emit('bf_init_state', (18 + 4 * 256,), words(array_body(bf, 'BF_init_state =')))

def ints(body):
    return [int(t, 0) for t in re.findall(r'\b(?:0x[0-9a-fA-F]+|\d+)\b', body)]

def cstr(src, decl):
    i = src.index(decl)
    m = re.compile(r'"([^"]*)"').search(src, i)
    return [ord(c) for c in m.group(1)]

bf_itoa = cstr(bf, 'BF_itoa64[64 + 1] =')
assert len(bf_itoa) == 64
emit('bf_itoa64', (64,), bf_itoa, 'uint8', '%d', 16)
emit('bf_atoi64', (0x60,), ints(array_body(bf, 'BF_atoi64[0x60] =')), 'uint8', '%d', 16)
emit('bf_flags_by_subtype', (26,), ints(array_body(bf, 'flags_by_subtype[26] =')), 'uint8', '%d', 26)

des_a64 = cstr(des, 'ascii64[] =')
assert len(des_a64) == 64
# The same alphabet in crypt_md5.c, crypt_sha256.c and crypt_sha512.c (b64).
for f in ('crypt_md5.c', 'crypt_sha256.c', 'crypt_sha512.c'):
    assert cstr(strip(open(d + '/' + f).read()), 'b64[] =') == des_a64, f
emit('crypt_ascii64', (64,), des_a64, 'uint8', '%d', 16)
emit('des_key_shifts', (16,), ints(array_body(des, 'key_shifts[16] =')), 'uint8', '%d', 16)

md5 = strip(open(d + '/crypt_md5.c').read())
emit('md5_tab', (64,), words(array_body(md5, 'tab[64] =')))
emit('md5_perm', (15,), ints(array_body(md5, 'perm[][3] =')), 'uint8', '%d', 15)
s256 = strip(open(d + '/crypt_sha256.c').read())
emit('sha256_k', (64,), words(array_body(s256, 'K[64] =')))
emit('sha256_perm', (30,), ints(array_body(s256, 'perm[][3] =')), 'uint8', '%d', 15)
s512 = strip(open(d + '/crypt_sha512.c').read())
emit('sha512_k', (80,), [int(t, 16) for t in re.findall(r'0x([0-9a-fA-F]+)ULL', array_body(s512, 'K[80] ='))],
     'uint64', '0x%016x_u64', 4)
emit('sha512_perm', (63,), ints(array_body(s512, 'perm[][3] =')), 'uint8', '%d', 15)
