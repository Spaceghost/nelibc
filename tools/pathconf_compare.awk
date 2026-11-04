BEGIN { FS = "," }
FNR == NR {
  if (FNR > 1 && $10 == "value") host[$4 SUBSEP $5 SUBSEP $6] = $7 SUBSEP $11
  next
}
FNR > 1 && $10 == "value" {
  # These values are implementation-specific and legitimately differ between
  # glibc and musl; retain them in the shootout CSV without requiring parity.
  # musl hardcodes 4096 for the transfer-size and allocation keys where glibc
  # and Nelibc report the filesystem's f_bsize, so they only agree on
  # filesystems with 4 KiB blocks.
  if (($6 == "LINK_MAX" && $2 == "musl") || $6 == "SYNC_IO" || $6 == "FILESIZEBITS" ||
      $6 == "REC_INCR_XFER_SIZE" || $6 == "REC_MAX_XFER_SIZE" ||
      ($2 == "musl" && ($6 == "REC_MIN_XFER_SIZE" || $6 == "REC_XFER_ALIGN" ||
                        $6 == "ALLOC_SIZE_MIN"))) next
  key = $4 SUBSEP $5 SUBSEP $6
  observed = $7 SUBSEP $11
  if (!(key in host) || host[key] != observed) {
    printf "pathconf mismatch for %s/%s/%s: host=%s nelibc=%s\n", $4, $5, $6, host[key], observed > "/dev/stderr"
    failed = 1
  }
}
END { exit failed }
