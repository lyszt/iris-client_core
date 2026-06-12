#!/usr/bin/env bash
# Runs INSIDE the deploy container, as a user that owns the GnuPG home.
# Builds the signed source package and uploads it. Not meant to be run directly.
set -euo pipefail

cd /b/eris
rm -rf build eris debian/eris dist obj-*

# Unique, monotonic snapshot version (Launchpad rejects duplicates).
VERSION="0.1.0+git$(date +%Y%m%d%H%M%S)"
dch -b -v "$VERSION" -D "$SERIES" "Snapshot build."

# Force the signing key when GPG_KEY is set; otherwise match by changelog email.
KEYOPT=()
[ -n "${GPG_KEY:-}" ] && KEYOPT=(-k"$GPG_KEY")

# -d: skip local build-dep check (source-only build; Launchpad installs deps).
debuild -S -sa -d "${KEYOPT[@]}"
dput "$PPA" "/b/eris_${VERSION}_source.changes"
