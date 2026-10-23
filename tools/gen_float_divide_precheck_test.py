#!/usr/bin/env python3
"""Generate a test-only Nelua source copy with an internal divider probe."""

from pathlib import Path
import sys

source_path = Path(sys.argv[1])
output_path = Path(sys.argv[2])
source = source_path.read_text()
anchor = "local function big_divide_round("
assert source.count(anchor) == 1
probe = r'''local function nelibc_test_big_divide_u64_precheck(low: culonglong,
    high: cuint, divisor: cuint, should_succeed: cint): cint
    <cexport, codename 'nelibc_test_big_divide_u64_precheck'>
  if divisor == 0 or high > 1 then return 1 end
  local numerator: BigUInt <noinit>
  numerator.limbs[0] = (@cuint)(low & 0xFFFFFFFF_culonglong)
  numerator.limbs[1] = (@cuint)(low >> 32)
  numerator.limbs[2] = high
  if high ~= 0 then numerator.used = 3
  elseif numerator.limbs[1] ~= 0 then numerator.used = 2
  elseif numerator.limbs[0] ~= 0 then numerator.used = 1
  else numerator.used = 0 end
  local index: csize = 0
  local carry: culonglong = 0
  while index < numerator.used do
    local product: culonglong =
      (@culonglong)(numerator.limbs[index]) * (@culonglong)(divisor) + carry
    numerator.limbs[index] = (@cuint)(product & 0xFFFFFFFF_culonglong)
    carry = product >> 32
    index = index + 1
  end
  if carry ~= 0 then
    if numerator.used == BIG_LIMBS then return 2 end
    numerator.limbs[numerator.used] = (@cuint)(carry)
    numerator.used = numerator.used + 1
  end
  local denominator: BigUInt <noinit>
  denominator.limbs[0] = divisor
  denominator.used = 1
  local original_used: csize = numerator.used
  local original_0: cuint = numerator.limbs[0]
  local original_1: cuint = numerator.limbs[1]
  local original_2: cuint = numerator.limbs[2]
  local original_denominator: cuint = denominator.limbs[0]
  local quotient: culonglong = 0
  local succeeded: boolean = big_divide_knuth_u64(&numerator, &denominator, &quotient)
  if should_succeed ~= 0 then
    if not succeeded or quotient ~= low or numerator.used ~= 0 then return 3 end
    local shift: cint = 0
    local top: cuint = divisor
    while (top & 0x80000000_cuint) == 0 do
      top = top << 1
      shift = shift + 1
    end
    if denominator.used ~= 1 or denominator.limbs[0] ~= (divisor << shift) then return 4 end
  else
    if succeeded then return 5 end
    if numerator.used ~= original_used or numerator.limbs[0] ~= original_0 or
        numerator.limbs[1] ~= original_1 or numerator.limbs[2] ~= original_2 or
        denominator.used ~= 1 or denominator.limbs[0] ~= original_denominator then
      return 6
    end
  end
  return 0
end

'''
capacity_probe = r'''local function nelibc_test_big_divide_capacity_precheck(): cint
    <cexport, codename 'nelibc_test_big_divide_capacity_precheck'>
  local numerator: BigUInt <noinit>
  local denominator: BigUInt <noinit>
  numerator.used = BIG_LIMBS
  denominator.used = BIG_LIMBS - 1
  local index: csize = 0
  while index < numerator.used do
    numerator.limbs[index] = (@cuint)(index ~ 0xA5A5A5A5_csize)
    index = index + 1
  end
  index = 0
  while index < denominator.used do
    denominator.limbs[index] = (@cuint)(index ~ 0x5A5A5A5A_csize)
    index = index + 1
  end
  numerator.limbs[BIG_LIMBS - 1] = 1
  denominator.limbs[BIG_LIMBS - 2] = 1
  local quotient: culonglong = 0
  if big_divide_knuth_u64(&numerator, &denominator, &quotient) then return 1 end
  if numerator.used ~= BIG_LIMBS or denominator.used ~= BIG_LIMBS - 1 then return 2 end
  index = 0
  while index < numerator.used do
    local expected: cuint = (@cuint)(index ~ 0xA5A5A5A5_csize)
    if index == BIG_LIMBS - 1 then expected = 1 end
    if numerator.limbs[index] ~= expected then return 3 end
    index = index + 1
  end
  index = 0
  while index < denominator.used do
    local expected: cuint = (@cuint)(index ~ 0x5A5A5A5A_csize)
    if index == BIG_LIMBS - 2 then expected = 1 end
    if denominator.limbs[index] ~= expected then return 4 end
    index = index + 1
  end
  return 0
end

'''
output_path.parent.mkdir(parents=True, exist_ok=True)
output_path.write_text(source.replace(anchor, probe + capacity_probe + anchor))
