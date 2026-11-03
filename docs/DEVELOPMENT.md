# Development build v0.2

This supersedes the initial source-only validation snapshot.

## Implemented

One map; eight original rounds; layered red through pink enemies; place/select/sell
Dart towers; First/Last/Strong targeting; money/lives; pause, 2x speed and diagnostics.
Dart upgrades now have three paths, each with three tiers:

| Key | Path | Tier effects | Costs |
| --- | --- | --- | --- |
| 1 | Range | 70 / 82 / 94 pixel range | 90 / 160 / 280 |
| 2 | Speed | 9 / 6 / 4 ticks between shots (base 12) | 120 / 220 / 400 |
| 3 | Power | 2 / 3 / 4 layers per impact (base 1) | 150 / 300 / 550 |

Select a tower, then press 1, 2 or 3 to buy. Sidebar shows key, current tier,
and next price; $0 means maxed or locked. At most two paths can be used, and
only one can reach tier 3. Selling returns floor(70% of total spending).
These are original prototype rules, not exact BTD6 upgrade statistics.

## Reproducible local build

From the project PowerShell terminal:

```powershell
python tools/build_native.py
python tools/build_native.py --tests
```

Or use VS Code Terminal > Run Task > CE: build game / CE: build engine tests.
The builder follows the installed CEdev v15 makefile's compile, assemble, partial
link, library-metadata, startup, final link and conversion stages. It uses
separate ignored object directories, no shell commands, no deletion and no
compression. LTO and shell icon generation are omitted in this build route.
Installed standard library and GRAPHX are still used, not bundled third-party
copies. The original make build remains available for environments supporting it.

## Measured build evidence

- LAYERCE.8xp: 23,551 bytes including transfer-file wrapper.
- Game BSS: 1,820 bytes; not a claim about total RAM consumed by libraries/stack.
- Linker-defined heap gap: 58,870 bytes before any allocations; not OS user free RAM.
- Native loader metadata: shared GRAPHX API minimum 14.
- TESTCE.8xp: 23,089 bytes. Additional tests cover upgrades, locking, refunds,
  slot reuse, insufficient funds and upgraded projectile damage.
- SHA-256: 43e5b3e5483598138b05a6c5b54eb6ce31dfa5afefad682bd2bedccabd531d01

C compilation and linking succeeded. The linker reports upstream missing
.note.GNU-stack metadata warnings; this is not a game-source compile error.
Both builds exist in bin/ and are ignored by Git. No FPS, gameplay acceptance,
stack high-water or physical-calculator result is claimed.

## First interactive test

1. Start the existing working CEmu ROM/configuration, at normal 100% speed.
2. Send bin/TESTCE.8xp through CEmu's Send files action.
3. Launch TESTCE through the existing working CEaShell/arTIfiCE launcher.
4. Record the test count and failures displayed. Require zero failures.
5. Send bin/LAYERCE.8xp and launch through that same working launcher.
6. Close help with 2nd, place near the path, select and try upgrade keys 1/2/3.
   Start rounds with Zoom; Mode pauses, Alpha changes targeting, Del sells,
   Trace shows diagnostics, Clear exits.
7. Send a screenshot of the first screen and TESTCE results. Complete the
   existing TESTING.md checklist before any physical-calculator transfer.

Direct command-line autotester launch has not established a completed engine
run. Its report marker stayed unset in the attempted launch; direct OS ASM
launch is a suspected blocker on this OS, not a demonstrated cause. The working
launcher needs an interactive check. The first test invocation also used an
incorrect CRC polynomial; only the later CRC32C invocation is relevant evidence.

Future expansion waits for baseline execution: tuned aiming/pierce, additional
bloon properties, then distinctive tower types and longer rounds. No save writes,
OS updates, resets, or calculator file deletion were performed.
