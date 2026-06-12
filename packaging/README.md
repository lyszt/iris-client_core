# Packaging Eris

Eris loads its `.pl` files relative to the binary, so packages install the binary +
`lib/` together and symlink `bin`:

```
/usr/lib/eris/eris
/usr/lib/eris/lib/...
/usr/bin/eris -> /usr/lib/eris/eris
```

## Local install

```bash
sudo make install                              # -> /usr/local
make install PREFIX=/usr DESTDIR=/tmp/stage    # staged, no root
sudo make uninstall
```

## AUR

`eris-cli-git` (`aur/`). Deps: `swi-prolog curl readline git`.

```bash
cd packaging/aur && makepkg -si
```
