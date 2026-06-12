# Debian / Ubuntu packaging

Packaging lives in `debian/` at the repo root (standard location). It reuses the
upstream `make install`, so the `.deb` gets the same layout as every other package:
binary + `.pl` tree under `/usr/lib/eris`, with `/usr/bin/eris` symlinked in.

## Build a .deb locally

On Debian/Ubuntu:

```bash
sudo apt install build-essential debhelper devscripts cmake pkg-config \
  libcurl4-openssl-dev libssl-dev libreadline-dev swi-prolog wget unzip

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

Set the target series in `debian/changelog` (the `noble` field) to whatever Ubuntu
release you're publishing for; upload one source per series.

Caveat: Launchpad builds offline. The build downloads libgit2 1.8.5, so for a PPA
either keep `vendor/` populated in the source tree (it is shipped — `tar-ignore`
only drops build junk) or switch CMake to the system `libgit2`.
