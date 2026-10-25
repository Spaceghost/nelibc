#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
  echo "usage: $0 standalone-spawn-smoke-binary" >&2
  exit 2
fi

if ! command -v unshare >/dev/null 2>&1; then
  echo "skip spawn RESETIDS namespace smoke: unshare is unavailable"
  exit 0
fi

user_name=$(id -un)
user_id=$(id -u)
group_id=$(id -g)
sub_uid=$(awk -F: -v name="$user_name" '$1 == name { print $2; exit }' /etc/subuid 2>/dev/null || true)
sub_gid=$(awk -F: -v name="$user_name" '$1 == name { print $2; exit }' /etc/subgid 2>/dev/null || true)
if [ -z "$sub_uid" ] || [ -z "$sub_gid" ]; then
  echo "skip spawn RESETIDS namespace smoke: subordinate uid/gid ranges are unavailable"
  exit 0
fi

if ! unshare --user \
    --map-users="0:$user_id:1" --map-users="1:$sub_uid:1" \
    --map-groups="0:$group_id:1" --map-groups="1:$sub_gid:1" \
    true >/dev/null 2>&1; then
  echo "skip spawn RESETIDS namespace smoke: user namespace mapping is unavailable"
  exit 0
fi

unshare --user \
  --map-users="0:$user_id:1" --map-users="1:$sub_uid:1" \
  --map-groups="0:$group_id:1" --map-groups="1:$sub_gid:1" \
  "$1"
unshare --user \
  --map-users="0:$user_id:1" --map-users="1:$sub_uid:1" \
  --map-groups="0:$group_id:1" --map-groups="1:$sub_gid:1" \
  "$1" --run-resetids
