#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT_DEFAULT="$(cd "$SCRIPT_DIR/.." && pwd)"

WIN_HOST="${WIN_HOST:-192.168.122.50}"
WIN_USER="${WIN_USER:-a}"
WIN_DEST="${WIN_DEST:-C:/Surreal64CE-B5.52}"
LOCAL_SRC="${LOCAL_SRC:-$REPO_ROOT_DEFAULT}"

SSH=(ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/tmp/win7_known_hosts)

echo "[1/3] Syncing source tree to ${WIN_DEST} (excluding xdk)..."
tar -C "$LOCAL_SRC" --exclude='xdk' -cf - . | \
  "${SSH[@]}" "${WIN_USER}@${WIN_HOST}" "C:\\cygwin\\bin\\tar.exe -xf - -C /cygdrive/c/Surreal64CE-B5.52"

echo "[2/3] Running Build.bat on Win7..."
"${SSH[@]}" "${WIN_USER}@${WIN_HOST}" \
  "cmd /c \"cd /d C:\\Surreal64CE-B5.52 && Build.bat > build.log 2>&1\"" || true

echo "[3/3] Last 80 lines of build.log:"
"${SSH[@]}" "${WIN_USER}@${WIN_HOST}" \
  "C:\\cygwin\\bin\\tail.exe -n 80 /cygdrive/c/Surreal64CE-B5.52/build.log"
