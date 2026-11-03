# Layer Defense CE — playable-prototype source, v0.1

**Setup first, development later:** start with [SETUP.md](SETUP.md). The workspace
now includes a compiler installer and a VS Code session backup task for the
`dev` branch. GitHub builds are manual. Compiler installation, GitHub sign-in
and the first remote push are still pending; see the setup guide.

A compact, original 2D tower defense project for the TI-84 Plus CE Python,
inspired by Bloons-style layer mechanics and strategic tower placement.
This is independent software, not a BTD6 port or a Ninja Kiwi product.

**Current validation:** source-data checks passed. C compilation, C engine tests,
CEmu execution, physical-calculator execution, FPS, and installed size are
**not yet verified**. There is no prebuilt `.8xp` in this initial source snapshot.
Do not describe this as a tested calculator release.

## What the source implements

- One original map with a 520-pixel orthogonal route.
- Freely placed dart towers, path/edge/overlap restrictions, selection, selling.
- Straight non-homing darts, collision detection, one layer popped per hit.
- Red, blue, green, yellow, pink layers; surviving lower layers slow down.
- First, Last, and Strong targeting (ties prefer farther-along enemies).
- Eight original rounds: 258 bloons, 766 layers in total.
- Money, life loss per remaining layer, round bonuses, defeat and victory.
- Pause, help, 1x/2x speed, clean exit, and a measurement screen.
- An isolated 30-second synthetic stress scenario with 24 towers and
  continuous refill to 96 bloons. This mode intentionally bypasses placement
  and economy rules; normal gameplay does not.

The source does not yet include upgrades, camo, regrow, splitting enemies,
MOAB classes, save files, more maps, or other tower classes. Those are staged
expansions, not forgotten requirements. Current wave and tower values are
original prototype balance, not exact BTD6 statistics.

## Your confirmed setup

You reported successful arTIfiCE and CEaShell launches on the physical calculator
and a working CEmu boot from your own ROM. These are **user-confirmed**, not
independently observed during development. Last reported OS is 5.8.4.0058,
with approximately 2,957 kB archive and 150 kB user RAM available.
The developer page supports arTIfiCE v2.1 through CE OS 5.8.4. Keep the working
setup; this project requires no OS update or reset. The downloaded OS/apps
bundle has not been inspected or authenticated by this project.

Before transferring experimental native code, back up important school files
with TI Connect CE. Test in CEmu first. Do not use a recovery bundle as a game
installation step.

## Build on Windows, step by step

All commands in this section run **on Windows**, not on the calculator.

1. Download **CEdev-Windows.zip, release v15.0** from the official project:
   https://github.com/CE-Programming/toolchain/releases/tag/v15.0
2. Extract it so the folder containing `bin` and `cedev.bat` is `C:\CEdev`.
   Another folder is fine, but its full path must contain no spaces.
   Installing this portable compiler does not change the calculator OS.
3. Open the `LAYERCE` folder in VS Code using **File > Open Folder**.
4. Choose **Terminal > New Terminal**. In a PowerShell terminal:

   ```powershell
   powershell -NoProfile -File .\tools\build.ps1 -CEdev C:\CEdev
   ```

   Or press **Ctrl+Shift+B**, choose **CE: build game**, and enter `C:\CEdev`.
   The script sets PATH only for its own process and retains `build.log`.
5. A successful build must produce `bin\LAYERCE.8xp` and a map in `bin`.
   The script prints the file size and SHA-256. A command being run is not
   proof of success: inspect the exit status and actual output file.

Alternative official toolchain method: double-click `C:\CEdev\cedev.bat`, then
in its **Command Prompt** window:

```bat
cd /d C:\Users\inf\Documents\Codex\2026-11-02\new-chat-2\LAYERCE
make V=1
```

Compiler errors: keep the full build log and map. Missing runtime DLLs may
require Microsoft's official Visual C++ Redistributable, as documented by CEdev.
Do not disable antivirus or make calculator OS changes to fix a build error.

## Required calculator dependencies

The expected shared dependency is **GRAPHX**, loaded through the CE library
loader. Keypad and standard runtime code are resolved by the toolchain;
inspect the actual build's dependency metadata and loader diagnostics before
declaring a complete list. **FONTLIBC and FILEIOC are not used by this source.**

Get libraries from the matching official release:
https://github.com/CE-Programming/libraries/releases/tag/v15.0

The release offers a group file and a separate-files ZIP. Prefer the separate
files when checking exactly what is missing. Your visible GRAPHX entry does
not establish its ABI version, and your screenshot is not a full inventory.
Test the existing libraries in CEmu first. If the loader reports a missing or
outdated dependency, install only the required compatible file after reviewing
the inventory. Do not blindly replace working libraries used by other programs.

CEaShell is a launcher already reported working; it is not linked into the game.
arTIfiCE v2.1 is the working OS-compatibility route, not a per-game asset.

## CEmu test first

These steps use **CEmu on Windows**.

1. Start your existing working ROM configuration. Save a separate emulator
   snapshot/state through CEmu's UI so testing does not overwrite your only
   useful emulator state. Never upload your ROM to GitHub.
2. Set emulator speed to **100%**, with normal throttling. An unthrottled
   emulator is useful for debugging but cannot establish physical FPS.
3. Use CEmu's **Send files** action to import `bin\LAYERCE.8xp` and any confirmed
   missing libraries. Use its command search/menu if labels vary by build.
4. Open the already-working arTIfiCE launcher or CEaShell inside the emulated
   calculator, select **LAYERCE**, and launch it. The help overlay should appear.
5. Run the checklist in [docs/TESTING.md](docs/TESTING.md), including the engine
   test program. Record measurements in [docs/RESULTS.md](docs/RESULTS.md).

## Physical calculator, only after emulator checks pass

1. Back up school files using TI Connect CE.
2. Send `bin\LAYERCE.8xp` with TI Connect CE, archived. Send only dependencies
   shown to be missing/incompatible by the preceding inventory and CEmu test.
3. **On the calculator**, open CEaShell or arTIfiCE and launch **LAYERCE**.
4. Start with a short normal round. Confirm movement, placement, pause,
   measurements, and CLEAR exit. Check that ordinary calculator calculations
   and your school programs still open normally.
5. Then run normal 1x, 2x, and the synthetic stress test. Do not infer hardware
   speed or RAM safety from a desktop-only pass.

## Controls on the calculator

| Key | Action |
| --- | --- |
| Arrows | Move placement cursor |
| 2nd or Enter | Place a dart tower for $180, or select a nearby tower |
| Zoom | Start next round when ready |
| Mode | Pause/resume; placement remains available while paused |
| + | Toggle 1x/2x simulation speed |
| Alpha | Cycle selected tower: First / Last / Strong |
| Del | Sell selected tower for $126 (70% of base cost) |
| Y= | Help/back; simulation suspends while an overlay is open |
| Trace | Measurement screen/back; simulation suspends |
| Window | Start stress test only from an untouched new game |
| Graph | New game after defeat, victory, or completed stress test |
| Clear | Exit from any screen; no save is written |

First attempt: close help with 2nd, place near the path, move to a second
position, place another tower, then press Zoom. You start with $500 and 100 lives.
Selecting an existing tower costs nothing. A blocked or unaffordable placement
does not deduct money. Holding a placement key does not repeatedly buy towers.

## Tests and build automation

Run data checks with existing Python, without extra packages:

```powershell
python .\tools\verify_data.py
```

This checks the actual source tables and integer bounds; **it does not run C**.

Build the real C engine tests for CEmu:

```powershell
powershell -NoProfile -File .\tools\build.ps1 -CEdev C:\CEdev -CoreTests
```

Import `bin\TESTCE.8xp` into CEmu and run through its launcher. It must report
zero failures. Build game and test variants separately: their object paths
overlap, so the wrapper runs `make clean` between builds.

If a Windows GCC or Clang host compiler is already available:

```powershell
powershell -NoProfile -File .\tools\test-host.ps1 -Compiler gcc
```

The test suite links the same `src/game.c`; it covers placement/economy,
targeting, layer rewards, life underflow, spawn backpressure, wave completion,
projectile saturation, campaign accounting, and stress-pool consistency.
Host tests do not reproduce the CE's 24-bit integer ABI or graphics driver.

The included, manually triggered GitHub Actions workflow is configured to run GCC engine tests
with AddressSanitizer/UndefinedBehaviorSanitizer, download pinned CEdev v15.0,
and build game/test `.8xp` artifacts and map evidence when explicitly started.
It has **not yet run**. A workflow file is not a successful build.
Use included GitHub Actions allowance only; do not enable paid usage.

## Design, budgets, and next milestones

See [docs/DESIGN.md](docs/DESIGN.md) for runtime accounting, architecture,
deliberate departures from BTD6, upgrade-tree plans, and expansion gates.
See [docs/SOURCES.md](docs/SOURCES.md) for primary documentation and provenance.

The game uses original geometric graphics generated by its own draw code.
No BTD6 images, sounds, names/logos as branding, ROMs, recovery bundles,
or third-party binaries belong in this repository.
