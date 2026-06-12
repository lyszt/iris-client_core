# Debian / Ubuntu packaging

Packaging lives in `debian/` at the repo root (standard location). It reuses the
upstream `make install`, so the `.deb` gets the same layout as every other package:
binary + `.pl` tree under `/usr/lib/eris`, with `/usr/bin/eris` symlinked in.

## Build a .deb locally

On Debian/Ubuntu:

```bash
sudo apt install build-essential debhelper devscripts cmake pkg-config \
  libgit2-dev libcurl4-openssl-dev libssl-dev libreadline-dev swi-prolog

dpkg-buildpackage -b -us -uc      # binary-only, unsigned
sudo apt install ../eris_0.1.0_amd64.deb
eris help
```

`-b` skips the source tarball, so build artifacts are a non-issue.

## PPA (Launchpad) — apt install for everyone

One-time: a Launchpad account, a GPG key registered there, and `dput` configured.

```bash
debuild -S -sa                    # signed source package
dput ppa:<you>/eris ../eris_0.1.0_source.changes
```

Set the target series in `debian/changelog` (the `resolute` field) to whatever Ubuntu
release you're publishing for; upload one source per series.

The build links the distro's `libgit2` (`libgit2-dev`), so nothing is fetched at
build time — Launchpad's offline builders are fine.
