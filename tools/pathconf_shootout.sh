#!/bin/sh
set -eu

host_binary=$1
nelibc_binary=$2
output_prefix=$3
shift 3

host_csv="${output_prefix}-host.csv"
nelibc_csv="${output_prefix}-nelibc.csv"
merged_csv="${output_prefix}.csv"
rm -f "$host_csv" "$nelibc_csv" "$merged_csv"

for target_path in "$@"; do
  host_part="${output_prefix}-host.part.csv"
  nelibc_part="${output_prefix}-nelibc.part.csv"
  "$host_binary" "$target_path" > "$host_part"
  "$nelibc_binary" "$target_path" > "$nelibc_part"
  if [ -s "$host_csv" ]; then
    tail -n +2 "$host_part" >> "$host_csv"
    tail -n +2 "$nelibc_part" >> "$nelibc_csv"
  else
    mv "$host_part" "$host_csv"
    mv "$nelibc_part" "$nelibc_csv"
  fi
done

{ cat "$host_csv"; tail -n +2 "$nelibc_csv"; } > "$merged_csv"
awk -f tools/pathconf_compare.awk "$host_csv" "$nelibc_csv"
cat "$merged_csv"
rm -f "${output_prefix}-host.part.csv" "${output_prefix}-nelibc.part.csv"
