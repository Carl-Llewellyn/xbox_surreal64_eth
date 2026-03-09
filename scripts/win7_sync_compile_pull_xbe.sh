#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT_DEFAULT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Win7 VM connection
WIN_HOST="${WIN_HOST:-192.168.122.50}"
WIN_USER="${WIN_USER:-a}"

# Local repo root and remote mirror path
LOCAL_ROOT="${LOCAL_ROOT:-$REPO_ROOT_DEFAULT}"
REMOTE_ROOT_CYG="${REMOTE_ROOT_CYG:-/cygdrive/c/Surreal64CE-B5.52}"
LOCAL_BUILD_DIR="${LOCAL_BUILD_DIR:-$LOCAL_ROOT/BUILD}"

# Build answers for Build.bat prompts
BUILD_MODE_ANSWER="${BUILD_MODE_ANSWER:-R}"   # D/P/F/R/L
DIST_ANSWER="${DIST_ANSWER:-y}"               # y/n

KNOWN_HOSTS="${KNOWN_HOSTS:-/tmp/win7_known_hosts}"
MUX="${MUX:-/tmp/win7_build_mux}"

SSH_BASE=(ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile="$KNOWN_HOSTS")
SSH_MUX=(ssh -S "$MUX" -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile="$KNOWN_HOSTS")

cleanup() {
  "${SSH_BASE[@]}" -S "$MUX" -O exit "${WIN_USER}@${WIN_HOST}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

echo "[1/5] Opening SSH control connection to ${WIN_USER}@${WIN_HOST} (password prompt once)..."
"${SSH_BASE[@]}" -M -S "$MUX" -o ControlPersist=20m -fN "${WIN_USER}@${WIN_HOST}"
"${SSH_BASE[@]}" -S "$MUX" -O check "${WIN_USER}@${WIN_HOST}" >/dev/null

echo "[2/5] Syncing source tree to Win7 (${REMOTE_ROOT_CYG})..."
# Exclude bulky SDK bundle; keep everything else needed for build.
tar -C "$LOCAL_ROOT" --exclude='xdk' -cf - . | \
  "${SSH_MUX[@]}" "${WIN_USER}@${WIN_HOST}" "C:\\cygwin\\bin\\tar.exe -xf - -C $REMOTE_ROOT_CYG"

echo "[3/5] Running Build.bat on Win7 (answers: ${BUILD_MODE_ANSWER}, ${DIST_ANSWER})..."
# Build.bat is interactive; prepare answer file and redirect full output to build.log.
"${SSH_MUX[@]}" "${WIN_USER}@${WIN_HOST}" \
  "cmd /c \"cd /d C:\\Surreal64CE-B5.52 && (echo ${BUILD_MODE_ANSWER}>answers.txt && echo ${DIST_ANSWER}>>answers.txt && echo.>>answers.txt) && Build.bat < answers.txt > build.log 2>&1\"" || true

echo "[4/5] Pulling full BUILD directory from Win7..."
rm -rf "$LOCAL_BUILD_DIR"
mkdir -p "$LOCAL_BUILD_DIR"
"${SSH_MUX[@]}" "${WIN_USER}@${WIN_HOST}" \
  "cmd /c \"\"C:\\cygwin\\bin\\bash.exe\" -lc \"cd /cygdrive/c/Surreal64CE-B5.52/BUILD && tar -cf - .\"\"" | \
  tar -C "$LOCAL_BUILD_DIR" -xf -

echo "[5/5] Done. Local BUILD directory synced to $LOCAL_BUILD_DIR"
find "$LOCAL_BUILD_DIR" -maxdepth 2 -type f | sed "s|^$LOCAL_ROOT/||" | sort

echo

echo "Last 40 lines of remote build.log:"
"${SSH_MUX[@]}" "${WIN_USER}@${WIN_HOST}" "C:\\cygwin\\bin\\tail.exe -n 40 /cygdrive/c/Surreal64CE-B5.52/build.log" || true
