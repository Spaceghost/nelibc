#!/usr/bin/env python3
"""Generate 24-bit limbs of 2/pi for binary80 Payne-Hanek reduction."""

from decimal import Decimal, localcontext

LIMBS = 700
BITS_PER_LIMB = 24


def main() -> None:
    bits = LIMBS * BITS_PER_LIMB
    # Chudnovsky contributes about 14 decimal digits per term. Keep more than
    # 100 guard digits beyond the requested 2/pi expansion before truncating.
    with localcontext() as ctx:
        ctx.prec = int(bits * 0.30103) + 120
        c = 426880 * Decimal(10005).sqrt()
        m, l, x, k = 1, 13591409, 1, 6
        total = Decimal(l)
        terms = ctx.prec // 14 + 2
        for i in range(1, terms):
            m = (k * k * k - 16 * k) * m // (i * i * i)
            l += 545140134
            x *= -262537412640768000
            total += Decimal(m * l) / Decimal(x)
            k += 12
        pi = c / total
        digits = int((Decimal(2) / pi) * (1 << bits))

    limbs = [(digits >> (BITS_PER_LIMB * i)) & 0xFFFFFF
             for i in reversed(range(LIMBS))]
    for i in range(0, LIMBS, 5):
        print("  " + ", ".join(f"0x{x:06X}_cuint" for x in limbs[i:i + 5]) + ",")


if __name__ == "__main__":
    main()
