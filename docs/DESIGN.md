# Design and resource plan

## Feasible goal

Preserve the decisions that make layered-bloon defense interesting: where to
place towers, which enemy to prioritize, how to spend money, which upgrade
branches to specialize, and how to answer enemy properties. Use original 2D
art and bounded waves. A full BTD6 port, all official content, and unlimited
late-game entity counts are not practical commitments for this device.

Use C and CEdev v15.0. Assembly is a later option for a profiled bottleneck.
Neither language nor compiler flags alone guarantee the best size or speed.
The user's expansion preference is **deeper upgrade choices before tower count**.

## Architecture in the source

| Component | Responsibility |
| --- | --- |
| game.h / game.c | Portable deterministic state and simulation; no graphics, filesystem, OS calls, floating point, or heap allocation |
| main.c | CE input, GraphX rendering, wall-clock scheduler, controls, measurement overlays |
| Wave tables | Compact layer/count/gap groups plus round bonuses; no per-enemy event script |
| Tests | Link the real engine separately from the calculator interface |

State is global/static, not a large stack-local Game. Bloons use Q4 path distance
and coordinates (16 fixed-point units per pixel). Route interpolation uses
orthogonal segments and integer arithmetic. There are no trigonometric or
square-root calculations in the update loop.

Target selection scans the bounded enemy pool only when a tower can fire.
A bounding-box rejection precedes squared-distance testing. Projectile collision
also rejects by bounding box before checking radius, and samples two half-steps.
That reduces tunneling at the current speed and radius; it is not a general
continuous-collision solver. Enemy movement precedes dart movement each tick;
full relative-motion sweeps are a possible later correctness improvement.

The first implementation intentionally measures a simple bounded scan rather
than adding a spatial index without evidence. Worst-case work still grows with
towers×bloons and darts×bloons. If busy-wave profiling warrants it, bucket bloons
by path segments or spatial cells, then verify identical targeting and popping.
Do not jump to assembly before reducing unnecessary candidate checks.

## Simulation and rendering

**Target, not a result:** 30 simulation steps/sec at 1x, 60 at 2x, rendering up
to 30 FPS. Q4 speeds and cooldowns are defined per simulation step, so fast-forward
executes more complete steps, rather than jumping enemies past collisions.

Wall-clock accumulation retains fractional timing using elapsed clock ticks
multiplied by SIM_HZ. At most four simulation steps execute per outer loop;
input is polled between batches. The backlog is capped at eight steps and an
individual wall-clock interval at 250 ms. Excess elapsed real time is discarded
and counted as a slow event. **Game time slows under sustained overload**;
simulation events are not skipped to catch up. A visible SLOW indicator and
actual steps/sec make this compromise observable. Input can still be delayed
by one expensive draw plus the bounded batch; responsiveness must be measured.

Full-screen GraphX drawing and page swaps are the initial baseline, with clipped
field drawing. No heap-sized background image is allocated. A later comparison
can measure dirty rectangles or lower render cadence while preserving simulation
timing. Dirty rectangles bring overlap/restoration complexity, so they are not
automatically a win on a crowded screen.

## Runtime memory — separate address regions

**Verified from current published CEdev documentation, not this build's map:**

| Region | Documented extent | Initial project target (not measured) |
| --- | ---: | --- |
| Code + initialized data + read-only data | Up to 64 KiB | At most 48 KiB including linked runtime; retain room for expansion |
| BSS + heap together | Shared roughly 60 KiB | Game and UI BSS under 8 KiB; no game heap allocation |
| Stack | Roughly 4 KiB | Keep call chain/local scratch under 1 KiB as a planning target; measure before claiming it |
| LCD VRAM | 153,600 bytes | Two 320×240 8-bit pages, 76,800 bytes each, used by GraphX |

The two graphics pages consume **VRAM**, not a second 153,600-byte allocation
inside the 60 KiB BSS/heap region. Never add them to the BSS figure again.
Do not count all of the displayed 150 kB user free RAM as a malloc budget.
`os_MemChk` is recorded only as OS/user free RAM, explicitly labeled **not heap**.
This build does not borrow that region or perform archive writes.

The default source pools are 96 bloons, 64 darts, and 24 towers. The packed field
sums are 7, 9, and 6 bytes respectively: 1,392 bytes total before ABI padding.
Game metadata, counters and UI add to that. **Do not call this the actual BSS:**
CE structure alignment and compiler/linker contributions need the build map.
TRACE displays target `sizeof(Game)`, `sizeof(Bloon)`, `sizeof(Dart)`, and
`sizeof(Tower)`; host values are not a substitute for target values.

The remaining BSS/heap space is reserve, not a request to allocate it all.
Library loader residency/relocations, linked code, and shell handoff behavior
must be reconciled against the installed v15 linker script, map and emulator
memory inspection. Published documentation alone does not establish their exact
residency for this build. Do not add library bytes to both code and heap; locate
them first. Initial deployment gate: no overlap, adequate stack headroom, loader
success with the user's inventory, and repeated clean exits.

## Archive/install accounting

Archive and runtime RAM are separate constraints. The source requests an
archived, **uncompressed** program. Measure before comparing compression modes;
compression can reduce stored bytes without reducing expanded execution size.

**Planning envelope only:** keep the initial incremental game program under
48 KiB, and use 64 KiB as a provisional all-new-files allowance including a
missing GRAPHX/loader dependency. This is a reservation, not a measured total
or a claim that an arbitrary existing library version is compatible.

For each build, fill this inventory using actual calculator variable sizes:

| Item | Incremental storage for user | Total setup footprint |
| --- | --- | --- |
| LAYERCE program, with embedded path/waves/UI | Actual variable size; `.8xp` file length also includes transfer wrapper | Same program counted once |
| GRAPHX and any loader-required AppVars | Zero if existing compatible copies; otherwise exact new/replacement delta | Count installed dependency once even when shared |
| CEaShell + its installed AppVars | Zero when unchanged | Inventory installed shell components once |
| arTIfiCE v2.1 program | Zero when unchanged | Inventory existing launcher once |
| Assets/maps/save | No separate files in v0.1 | Embedded assets/maps already counted; no save |
| TESTCE | Optional temporary developer artifact | Count separately if kept installed |

FONTLIBC is already visible, but the game does not require it. Do not count it
as an additional install. Count it in the overall calculator inventory only if
reporting all installed software. CEmu, CEdev, this source repo, and the ROM dump
are PC files and consume no calculator archive unless separately transferred.

## Deliberate current simplifications

- A colored bloon becomes one lower layer; no branching children yet.
- Darts have one hit, finite lifetime, no pierce and no homing. They aim at the
  current location, with approximate max-axis-normalized velocity; diagonal
  shots are faster than axial shots. Lead aim/lookup directions need comparison.
- Speeds, prices, wave recipes and bonuses are original balancing values.
- Enemy cap: defer new spawns and show wait counters, never delete an enemy.
- Dart cap: delay firing and retry next step without starting the fire cooldown.
- No in-game saving or archive writes. Exiting loses the current run.

These are explicit v0.1 choices, not promises of exact BTD6 behavior.

## Milestones toward substantial strategic depth

| Milestone | Deliverable | Acceptance gate |
| --- | --- | --- |
| 0 — current source | Dart prototype, eight rounds, diagnostics, tests, build scripts | Actual CE compile + zero engine test failures + CEmu playthrough; this gate is still pending |
| 1 — measured baseline | Physical 1x/2x timing, size/map inventory, tuned collisions/balance | Several normal rounds near 30 FPS target; 1x SPS near 30, 2x near 60; no invalid state or crashes; measured input latency acceptable |
| 2 — Dart depth | Three upgrade paths, initially three tiers each; data-driven prices and effects | Main path may exceed tier 2, one secondary capped at 2, third locked; test purchase/sell/targeting invariants |
| 3 — enemy counters | Camo detection, regrow with cooldown, branching bloons; one MOAB class and children | Atomic or reserved-capacity splits; no lost children, regrow farming exploits, or cap-induced crashes |
| 4 — roster | Tack and Sniper, then Bomb and Ninja if they add distinct decisions | Profile explosion/area/pierce work; preserve a clear upgrade niche for each tower |
| 5 — campaign | Longer original rounds, difficulty settings, another map, meaningful progression | Cap-aware wave design and full campaign regression; no unrestricted endless-mode promise |
| 6 — reliable saves | Versioned explicit serialization, checksum, validate-before-load, two-slot save strategy | Simulated interrupted write/corrupt/old saves handled; add FILEIOC only if benefits justify dependency |

Three paths with three tiers is a proposed initial structure, based on the user's
preference. It preserves specialization and crosspath decisions cheaply in tower
state, but each upgrade still needs distinct behavior, UI, art and balancing.
Full three-path five-tier breadth is a later evaluation, not a hidden commitment.
The final game should have fewer well-differentiated choices before it has many
near-identical towers.

For a splitting enemy, reserve slots before transforming or use a bounded
pending-child queue tied to its parent. Defer the transformation if necessary;
never simply discard excess children. Extend tests before shipping that mechanic.
