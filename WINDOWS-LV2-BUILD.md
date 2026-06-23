# Windows LV2 Build

The Vox Continental LV2 plugin lives under `lv2/bristol-vox` and has its own
Windows Makefile. Build it from an MSYS2 UCRT64 shell.

## Prerequisites

```sh
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-pkg-config \
  mingw-w64-ucrt-x86_64-lv2 \
  mingw-w64-ucrt-x86_64-winpthreads
```

## Build And Install

```sh
cd /c/Users/chica/git/bristol/lv2/bristol-vox
make -f Makefile.win
make -f Makefile.win install
```

The install target copies the bundle to:

```text
~/.lv2/bristol-vox.lv2
```

The installed bundle should contain:

```text
bristol-vox.dll
manifest.ttl
```

If your LV2 host does not scan `~/.lv2` automatically, add it to `LV2_PATH`
before launching the host:

```sh
export LV2_PATH="$HOME/.lv2${LV2_PATH:+:$LV2_PATH}"
```
