#!/usr/bin/env bash
# scripts/release.sh — build and publish a GitHub release for zPen.
#
# The version is read from debian/changelog (single source of truth). Bump it
# with `dch -v <new-version>-1 "..."` (or edit the file by hand) before running
# this script.
#
# Usage:
#   scripts/release.sh             build, confirm, tag, push, publish
#   scripts/release.sh --dry-run   build only; skip tag, push, and release
#   scripts/release.sh --yes       skip the confirmation prompt
#
# Required tools: gh, dpkg-buildpackage, dpkg-parsechangelog, git.

set -euo pipefail

DRY_RUN=0
SKIP_CONFIRM=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run)  DRY_RUN=1 ;;
    --yes|-y)   SKIP_CONFIRM=1 ;;
    -h|--help)  sed -n '2,/^$/p' "$0" | sed 's/^# \?//'; exit 0 ;;
    *) echo "Unknown arg: $1" >&2; exit 2 ;;
  esac
  shift
done

cd "$(git rev-parse --show-toplevel)"

# --- preflight ---
for tool in gh dpkg-buildpackage dpkg-parsechangelog git; do
  command -v "$tool" >/dev/null \
    || { echo "ERROR: $tool not installed" >&2; exit 1; }
done

# Confirm gh is authenticated. Without this, the script would happily push the
# tag and then fail at the very last step, leaving a tag on origin with no
# matching Release object — you'd then need to recover by hand.
if ! gh auth status >/dev/null 2>&1; then
  echo "ERROR: gh is not authenticated. Run: gh auth login" >&2
  exit 1
fi

# Reject modified tracked files; untracked is fine (the .deb only ships HEAD).
if [[ -n "$(git status --porcelain | grep -v '^??' || true)" ]]; then
  echo "ERROR: working tree has uncommitted changes. Commit or stash first." >&2
  git status --short
  exit 1
fi

# --- compute version + tag ---
PKG_VERSION=$(dpkg-parsechangelog -SVersion)
UPSTREAM_VERSION=${PKG_VERSION%-*}
TAG="v${UPSTREAM_VERSION}"
ARCH=$(dpkg --print-architecture)
DEB_NAME="zpen_${PKG_VERSION}_${ARCH}.deb"
DEB_PATH="../${DEB_NAME}"
# Stable-named copy for the GitHub "latest release" redirect URL. Lets the
# README ship one permanent link that always resolves to the newest .deb.
DEB_LATEST_NAME="zpen_latest_${ARCH}.deb"
DEB_LATEST_PATH="../${DEB_LATEST_NAME}"

# Pull the body of the latest changelog entry — everything between the
# `zpen (...)` header and the ` -- maintainer` trailer.
NOTES=$(awk '
  /^zpen \(/ && !found { found=1; next }
  /^ -- / && found     { exit }
  found && NF          { sub(/^  /,""); print }
' debian/changelog)

cat <<EOF
============================================================
Version : ${UPSTREAM_VERSION}
Package : ${PKG_VERSION}
Tag     : ${TAG}
Arch    : ${ARCH}
Artifact: ${DEB_PATH}

Notes:
${NOTES}
============================================================
EOF

# --- ensure tag doesn't already exist ---
if git rev-parse --verify --quiet "refs/tags/${TAG}" >/dev/null; then
  echo "ERROR: tag ${TAG} already exists locally. Bump the version in debian/changelog." >&2
  exit 1
fi
if git ls-remote --exit-code --tags origin "${TAG}" >/dev/null 2>&1; then
  echo "ERROR: tag ${TAG} already exists on origin. Bump the version in debian/changelog." >&2
  exit 1
fi

# --- build ---
echo
echo "Building .deb..."
make clean >/dev/null
dpkg-buildpackage -us -uc -b
[[ -f "${DEB_PATH}" ]] \
  || { echo "ERROR: build did not produce ${DEB_PATH}" >&2; exit 1; }
cp -f "${DEB_PATH}" "${DEB_LATEST_PATH}"
echo "Built: ${DEB_PATH}"
echo "       ${DEB_LATEST_PATH} (stable alias)"

if (( DRY_RUN )); then
  echo
  echo "[--dry-run] Skipping tag, push, and GitHub release."
  exit 0
fi

# --- confirm ---
if ! (( SKIP_CONFIRM )); then
  read -r -p "Proceed with tag, push, and GitHub release? [y/N] " ANSWER
  [[ "${ANSWER}" =~ ^[Yy]$ ]] || { echo "Aborted."; exit 1; }
fi

# --- tag, push, release ---
git tag -a "${TAG}" -m "zPen ${UPSTREAM_VERSION}"
git push origin "${TAG}"

gh release create "${TAG}" "${DEB_PATH}" "${DEB_LATEST_PATH}" \
  --title "zPen ${UPSTREAM_VERSION}" \
  --notes "${NOTES}"

echo
echo "Release published:"
gh release view "${TAG}" --json url --jq .url
