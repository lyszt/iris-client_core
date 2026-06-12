#!/usr/bin/env bash
# Build the Eris .deb inside an Ubuntu container (for building from non-Debian hosts).
# Output lands in ./dist/ at the repo root. Usage: packaging/deb/build-in-docker.sh [ubuntu-tag]
set -euo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
IMAGE="ubuntu:${1:-26.04}"
OUT="$REPO/dist"
mkdir -p "$OUT"

docker run --rm \
  -v "$REPO:/src:ro" \
  -v "$OUT:/out" \
  "$IMAGE" bash -euo pipefail -c '
    export DEBIAN_FRONTEND=noninteractive
    apt-get update -qq
    apt-get install -y --no-install-recommends \
      build-essential debhelper devscripts cmake pkg-config \
      libgit2-dev libcurl4-openssl-dev libssl-dev libreadline-dev swi-prolog
    mkdir -p /work && cp -r /src /work/eris && cd /work/eris
    rm -rf build eris debian/eris dist
    dpkg-buildpackage -b -us -uc
    cp /work/*.deb /out/
  '

echo
echo "Built:"
ls -1 "$OUT"/*.deb
