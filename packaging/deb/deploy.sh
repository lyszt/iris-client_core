#!/usr/bin/env bash
# Build a signed Debian source package and upload it to a Launchpad PPA.
# Launchpad then builds the .deb for `apt install`. Runs inside an Ubuntu
# container, signing with the host's GnuPG key, so it works from any host.
#
#   PPA=ppa:lyszt/eris SERIES=resolute packaging/deb/deploy.sh
#
# One-time setup:
#   - a Launchpad account with a PPA created,
#   - your GPG key uploaded to Launchpad, registered to the changelog email,
#   - that secret key present in ~/.gnupg here.
set -euo pipefail

PPA="${PPA:-ppa:lyszt/eris}"
SERIES="${SERIES:-resolute}"
IMAGE="ubuntu:${UBUNTU_TAG:-26.04}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"

if [ ! -d "$HOME/.gnupg" ]; then
  echo "No ~/.gnupg found — a GPG signing key is required for a PPA upload." >&2
  exit 1
fi

echo "Deploying to $PPA (series: $SERIES) ..."
docker run --rm -it \
  -v "$REPO:/src:ro" \
  -v "$HOME/.gnupg:/root/.gnupg" \
  -e PPA="$PPA" -e SERIES="$SERIES" \
  "$IMAGE" bash -euo pipefail -c '
    export DEBIAN_FRONTEND=noninteractive
    export DEBEMAIL="luis.almeida@litessera.com"
    export DEBFULLNAME="João Luís Almeida Santos"
    export GPG_TTY=/dev/console
    apt-get update -qq
    apt-get install -y --no-install-recommends \
      devscripts dput dpkg-dev debhelper gnupg ca-certificates \
      distro-info-data libdistro-info-perl

    mkdir -p /b && cp -r /src /b/eris && cd /b/eris
    rm -rf build eris debian/eris dist obj-*

    # Unique, monotonic snapshot version per upload (Launchpad rejects
    # duplicates). +git sorts ABOVE the 0.1.0 base, so dch does not warn.
    VERSION="0.1.0+git$(date +%Y%m%d%H%M%S)"
    dch -b -v "$VERSION" -D "$SERIES" "Snapshot build."

    # -d: skip the local build-dep check. This builds only the SOURCE package;
    # Launchpad installs the build-deps when it compiles the binary.
    debuild -S -sa -d
    dput "$PPA" "/b/eris_${VERSION}_source.changes"
  '
echo "Uploaded. Launchpad will email you the build result; then:"
echo "  sudo add-apt-repository $PPA && sudo apt update && sudo apt install eris"
