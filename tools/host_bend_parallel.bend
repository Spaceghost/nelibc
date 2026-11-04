import Base

# Parallel calls: the runtime opens its worker pool for the fork-join tree,
# so under the host profile the workers run as lanes. pow2(16n) is 65536.
def pow2(+n: Nat) -> U32:
  match n:
    case 0n:
      1
    case 1n+p:
      a b = pow2(p) pow2(p)
      (a + b : U32)

def main() -> IO(Unit):
  IO.print(U32.show(pow2(16n)))
