# Bristol Vox LV2

Headless **Vox Continental** emulator from Bristol as an LV2 plugin.

| Platform | Build | Test host |
|----------|-------|-----------|
| **Windows** | `Makefile.win` / `build-windows.ps1` | [Carla](https://kxstudio.studio/Applications:Carla) |
| **Linux / Pi** | `Makefile` | mod-host, Carla, Ardour |
| **Linux → Windows** | `make -f Makefile.win CROSS=x86_64-w64-mingw32` | copy `.dll` bundle to Windows |

No GUI. MIDI notes plus drawbar / gain / vibrato controls.

---

## Windows (recommended for pre-Pi testing)

mod-host is Linux-only. On Windows, use **Carla** as the LV2 host.

### 1. Install MSYS2

Download from [msys2.org](https://www.msys2.org/) (default: `C:\msys64`).

Open **MSYS2 UCRT64** and install the toolchain:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make \
    mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-lv2 \
    mingw-w64-ucrt-x86_64-winpthreads
```

### 2. Build

**From PowerShell** (in `lv2/bristol-vox`):

```powershell
.\build-windows.ps1
.\build-windows.ps1 -Install   # copies to %USERPROFILE%\.lv2\
```

**Or from the UCRT64 shell:**

```bash
cd /e/path/to/bristol/lv2/bristol-vox
make -f Makefile.win
make -f Makefile.win install
```

Output bundle:

```
bristol-vox.lv2/
  manifest.ttl      (all plugin metadata + ports)
  bristol-vox.dll   (Windows) / bristol-vox (Linux)
  bristol-vox.dll
```

### 3. Automated tests

Build and run the headless test suite (engine API + LV2 lifecycle):

```powershell
.\build-windows.ps1 -Test
```

Or from MSYS2 UCRT64:

```bash
make -f Makefile.win test
```

On Linux / Pi:

```bash
make test
```

Tests cover engine init/fini, MIDI, drawbars, ring-buffer regression, block-size changes, and full `instantiate` → `run` → `cleanup` via `lv2_descriptor()`.

### 4. Test in Carla (Windows)

Install [Carla](https://kxstudio.studio/Applications:Carla) (64-bit). LV2 path: `%USERPROFILE%\.lv2` (after `.\build-windows.ps1 -Install`).

**Use Patchbay mode on Windows.** Continuous Rack is simpler but has a [known Windows audio bug](https://github.com/falkTX/Carla/issues/1065) (meters move, no sound). Patchbay mode works reliably with DirectSound.

#### Clean Patchbay setup (do this in order)

1. **Settings → Engine**
   - Audio driver: **DirectSound**
   - Process mode: **Patchbay** (not Continuous Rack)
   - Sample rate: **44100**, buffer: **512**
   - Click OK, then **Engine → Restart**
2. **Rack** tab → **Remove All** (clear any plugins added in the wrong mode)
3. **Engine → Start**
4. **Add Plugin → Bristol Vox** (must be added *after* Patchbay mode is active)
5. **Patchbay** tab → **Canvas → Show Internal**

You should see **two** boxes (not one):

| Box | Ports |
|-----|--------|
| **Bristol Vox** | red `midi_in`, blue `audio_l` / `audio_r` |
| **Audio Output** (Carla master) | blue inputs |

Wire **Internal**:

- `Carla midi-in` (or `events in`) → **Bristol Vox `midi_in`**
- **Bristol Vox `audio_l`** → **Audio Output** left
- **Bristol Vox `audio_r`** → **Audio Output** right

Then **Canvas → Show External** and wire:

- **loopMIDI Port** (or VMPK port) → **Carla `midi-in`**
- **Carla `audio-out1`** → **Playback Left**
- **Carla `audio-out2`** → **Playback Right**

Play VMPK — DSP should rise above 0%.

#### If Bristol never appears in Show Internal

This is a Carla graph issue, not proof the plugin is broken.

- In **Continuous Rack**, plugins **never** show as their own internal box — only Carla’s master I/O. That is normal; use Patchbay instead.
- If you switched from Continuous Rack to Patchbay, the old session is stale: **Remove All**, restart engine, re-add Bristol (steps above).
- **Canvas → Refresh**, zoom out (mouse wheel), check the minimap — the box may be off-screen.
- On **Rack**, confirm Bristol has the green power icon on.
- Sanity check: add another LV2 (e.g. MDA Piano). If *no* plugins appear in Internal, the Carla session/driver is the problem.

#### Alternative: jalv (bypass Carla)

In the MSYS2 **UCRT64** shell:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-jalv
export LV2_PATH=$USERPROFILE/.lv2
jalv "http://bristol.sourceforge.net/lv2/vox"
```

Use VMPK → loopMIDI → jalv’s MIDI input. This confirms the plugin before fighting Carla routing.

Environment variable (optional):

```cmd
set LV2_PATH=%USERPROFILE%\.lv2;%CD%\bristol-vox.lv2
```

---

## Cross-compile Windows DLL from Linux

Useful if your dev machine is Linux but you want to test the `.dll` on a Windows box.

```bash
sudo apt install mingw-w64 pkg-config

# Copy or mount MinGW LV2 headers (from MSYS2: /ucrt64/include/lv2)
# Or install a mingw lv2 package if your distro provides one.

cd lv2/bristol-vox
make -f Makefile.win CROSS=x86_64-w64-mingw32 \
    LV2_CFLAGS="-I/usr/x86_64-w64-mingw32/include"
```

Copy the entire `bristol-vox.lv2` folder to Windows and load in Carla.

---

## Linux / Raspberry Pi (mod-host)

```bash
sudo apt install build-essential pkg-config lv2-dev
cd lv2/bristol-vox
make
sudo make install
```

mod-host + JACK:

```bash
jackd -dalsa -r44100 -p128 -n2 &
export LV2_PATH=$HOME/.lv2
mod-host
# add "http://bristol.sourceforge.net/lv2/vox" 0
```

For MOD devices, cross-compile with [mod-plugin-builder](https://github.com/moddevices/mod-plugin-builder) using the Linux `Makefile`.

### CI (GitHub Actions)

On push/PR, [`.github/workflows/bristol-vox-lv2.yml`](../../.github/workflows/bristol-vox-lv2.yml) builds on **native ARM64** inside a **Debian Bookworm** container (glibc 2.36, matching Raspberry Pi OS) and uploads the LV2 bundle.

Download the **bristol-vox-lv2-linux-arm64** artifact from a green workflow run, then on the Pi:

```bash
mkdir -p ~/.lv2/bristol-vox.lv2
cp bristol-vox manifest.ttl ~/.lv2/bristol-vox.lv2/
export LV2_PATH=$HOME/.lv2
mod-host   # or Carla / jalv
```

Requires **Raspberry Pi OS 64-bit** (Pi 4/5).

---

## Controls

| Port | Function |
|------|----------|
| `midi_in` | Note on/off |
| `drawbar_16`, `drawbar_8`, `drawbar_4` | Footage drawbars (0–1) |
| `drawbar_iv` | IV footage |
| `drawbar_flute` | **Flute mix bus** — must be up for tonewheel footages to sound (default 1.0) |
| `drawbar_reed` | **Reed mix bus** — optional rasp layer (default 0) |
| `gain` | Master level |
| `vibrato` | Toggle |
| `audio_l` / `audio_r` | Stereo output |

---

## Limitations

- Vox Continental only (not Vox M2)
- Single instance (Bristol global state in `vox.c`)
- GPL v3+

---

## Troubleshooting (Windows)

| Problem | Fix |
|---------|-----|
| `pkg-config lv2` not found | Install `mingw-w64-ucrt-x86_64-lv2` in MSYS2 |
| Carla does not list plugin | Check `LV2_PATH`; bundle must contain `.ttl` + `.dll` |
| Error 193 / not valid Win32 app | Rebuild with latest `.\build-windows.ps1 -Install` (pthread is now linked statically). Use **64-bit Carla**. LV2 path: `%USERPROFILE%\.lv2` |
| Bristol in list but name empty / all counts zero | Carla only read the stub `manifest.ttl` and ignored `plugin.ttl`. Reinstall (`.\build-windows.ps1 -Install`) — metadata is now in one `manifest.ttl`. Remove any extra LV2 path pointing at the source tree; use only `%USERPROFILE%\.lv2`. Then **Configure → Refresh**. |
| External MIDI wired, DSP stays 0% | Internal wiring missing: Bristol must be connected inside Carla (midi-in → Bristol → audio output). |
| MIDI works but silent until Flute/Reed moved | Flute/Reed are **mix buses**. Carla often keeps Flute at 0 (saved rack or ignored TTL default); the plugin now keeps the flute bus on whenever footages are up. Close Carla before `.\build-windows.ps1 -Install` if the DLL is locked. |
| Continuous Rack: meters move, no sound | [Known Carla Windows bug](https://github.com/falkTX/Carla/issues/1065). Switch to **Patchbay** mode. |
| `bzero` / `unistd` errors | Ensure `bristol_lv2_platform.h` is included (`Makefile.win`) |
| Link errors for `pthread` | Install `mingw-w64-ucrt-x86_64-winpthreads` |

Paste full `make` output if the build fails and we can add missing stubs.
