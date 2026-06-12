#!/usr/bin/env bash
# Build a signed Debian source package and upload it to a Launchpad PPA.
# Launchpad then builds the .deb for `apt install`. Runs inside an Ubuntu
# container, signing with the host's GnuPG key, so it works from any host.
#
#   PPA=ppa:lyszt/eris SERIES=resolute packaging/deb/deploy.sh
#   GPG_KEY=<keyid> ...        # optional: force the signing key
#
# One-time setup:
#   - a Launchpad account with a PPA created,
#   - your GPG key uploaded to Launchpad, registered to the changelog email,
#   - that secret key present in ~/.gnupg here.
set -euo pipefail

PPA="${PPA:-ppa:lyszt/eris}"
SERIES="${SERIES:-resolute}"
GPG_KEY="${GPG_KEY:-}"
IMAGE="ubuntu:${UBUNTU_TAG:-26.04}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
HOST_UID="$(id -u)"
HOST_GID="$(id -g)"

if [ ! -d "$HOME/.gnupg" ]; then
  echo "No ~/.gnupg found — a GPG signing key is required for a PPA upload." >&2
  exit 1
fi

echo "Deploying to $PPA (series: $SERIES) ..."
docker run --rm -it \
  -v "$REPO:/src:ro" \
  -v "$HOME/.gnupg:/gnupg-src:ro" \
  -e PPA="$PPA" -e SERIES="$SERIES" -e GPG_KEY="$GPG_KEY" \
  -e HOST_UID="$HOST_UID" -e HOST_GID="$HOST_GID" \
  "$IMAGE" bash -euo pipefail -c '
    export DEBIAN_FRONTEND=noninteractive
    apt-get update -qq
    apt-get install -y --no-install-recommends \
      devscripts dput dpkg-dev debhelper gnupg pinentry-curses \
      distro-info-data libdistro-info-perl ca-certificates

    # Sign as a user owning the GnuPG home — avoids gpg "unsafe ownership"
    # (which hides the secret key when the host uid != container root). Reuse
    # whatever user already holds the host uid (Ubuntu images ship one at 1000).
    if [ "$HOST_UID" = "0" ]; then
      U=root; H=/root
    else
      EXISTING="$(getent passwd "$HOST_UID" | cut -d: -f1 || true)"
      if [ -n "$EXISTING" ]; then
        U="$EXISTING"; H="$(getent passwd "$HOST_UID" | cut -d: -f6)"
      else
        groupadd -g "$HOST_GID" builder 2>/dev/null || true
        useradd -m -u "$HOST_UID" -g "$HOST_GID" builder
        U=builder; H=/home/builder
      fi
    fi
    mkdir -p "$H"

    mkdir -p /b && cp -r /src /b/eris
    rm -rf "$H/.gnupg"
    cp -a /gnupg-src "$H/.gnupg"
    find "$H/.gnupg" \( -name "*.lock" -o -name ".#lk*" -o -name "S.*" \) -delete
    chmod 700 "$H/.gnupg"
    chown -R "$HOST_UID:$HOST_GID" /b "$H" "$H/.gnupg"

    # C.UTF-8 so the maintainer name is not mangled during signing.
    runuser -u "$U" -- env \
      HOME="$H" GNUPGHOME="$H/.gnupg" \
      LANG=C.UTF-8 LC_ALL=C.UTF-8 \
      DEBEMAIL="luis.almeida@litessera.com" \
      DEBFULLNAME="João Luís Almeida Santos" \
      PPA="$PPA" SERIES="$SERIES" GPG_KEY="$GPG_KEY" \
      bash /b/eris/packaging/deb/_ppa-inner.sh
  '
echo "Uploaded. Launchpad will email you the build result; then:"
echo "  sudo add-apt-repository $PPA && sudo apt update && sudo apt install eris"
