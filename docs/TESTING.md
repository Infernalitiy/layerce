# Repeatable test protocol

## Gate A — compile and engine correctness

1. Run `python tools/verify_data.py`. Expected route 520 px, 8 rounds, 13 groups,
   258 bloons, 766 layers. This is not execution testing.
2. Run the host C tests if a host compiler exists. In CI they run with ASan/UBSan.
3. Build TESTCE and run it in CEmu; require zero failures. Record the exact check
   count printed, rather than copying a hypothetical number into results.
4. Build LAYERCE separately; retain `.map`, `.8xp` size, SHA-256 and complete log.
5. In CEmu inspect the loaded program and library regions, BSS/heap boundaries,
   and stack high-water under the heaviest scenario. Confirm the actual installed
   linker script layout rather than assuming the documentation is exact.

## Gate B — interactive emulator acceptance

Use a fresh run and normal 100% emulator speed, with throttling enabled.

- Help appears and suspends simulation. Close it; arrows move the cursor.
- Place at the initial cursor (96,120): money changes $500 → $320.
- Press/release 2nd again without moving: selects, does not charge again.
- Hold 2nd: no repeated purchases. Select, Alpha cycles First/Last/Strong.
- Attempt placement on the path or too close to another tower: no charge.
- Select and Del: refund $126 once. Repeated Del cannot create more money.
- Place near (30,38) and (96,120), then Zoom. Bloons traverse every corner;
  observe darts and visible lower colors after hits. Adjust defense as needed.
- Mode: positions and round progression freeze; placement still works.
- Y= or Trace: state freezes; closing resumes without a catch-up burst.
- +: toggles speed. Do not infer correctness from faster rendering alone;
  record the SPS counter and compare fixed game-time behaviors.
- With no defenses, leaks subtract remaining layers; no unsigned-life wrap.
- Every round ends only after all groups have spawned and all enemies resolve.
  Completing round 8 shows victory; defeat stops further simulation.
- Clear exits from help, active wave, paused state and diagnostics. Launch again
  and repeat at least five times. Run a normal calculation after each exit.
- No new save/AppVars are created by the game. Existing school files remain.

## Gate C — performance

Normal scene: start a new run, place two early towers near the route, run rounds
1–3. Record each round at 1x and 2x. Later capture a developed run with more towers.
Do not compare different rounds/layouts as if speed alone changed.

Synthetic scene: launch fresh, leave the game untouched, then press Window from
help. It uses 24 towers and continuously refills to 96 enemies for 30 wall seconds.
Do not pause/open overlays during this interval: the deadline is wall time.
At the end it freezes diagnostics. Record the displayed counters and Graph for
a new run. Repeat three times in CEmu, then three times on the physical calculator.
Stress timing includes refill overhead. Projectile count is workload-dependent;
this is not a promise that all 64 dart slots stay occupied.

TRACE values:

| Value | Meaning |
| --- | --- |
| FPS | Completed frame submissions/sec over last approximately one-second window; includes GraphX drawing wait time, not optical LCD latency |
| SPS | Actual completed simulation ticks/sec; expect about 30/60 at 1x/2x if sustained |
| SIM avg/max | Average over last sampled window and max sampled step since run began, in `clock()` ticks |
| DRAW avg/max | Same for scene drawing plus swap submission; next-frame GraphX wait is included |
| Game / B/D/T | Actual target sizeof values, not a free-RAM reading |
| OS free at launch | os_MemChk result; not native heap availability |
| B/D peaks | Peak concurrent bloons and darts |
| Slow events | Wall interval/backlog cap events; indicates real-time slowdown |
| Pool waits B/D | Delayed enemy spawns / attempted shots because a pool was full |
| Spawn / kill / leak / pop | Accounting: spawn = kill + leak + active; pops count layers |

Convert clock ticks to milliseconds with `ticks * 1000 / displayed clock Hz`.
The averages use integer clock ticks, so short operations can quantize to zero.
Maxima reflect active sampling only. Pause/overlay drawing is excluded from
sample counters; results freeze in diagnostics. Read CPU time and real-time SPS
together. Low FPS alone does not prove simulation is incorrect.

For control latency, use phone slow-motion footage if useful: measure key press
to cursor/pause response in the busiest scenario, then subtract/consider capture
frame uncertainty. A proposed usability target is under 100 ms; it is not measured.

## Release decisions

Proposed initial goals: ordinary play close to 30 FPS; 1x simulation sustained
near 30 SPS; 2x near 60 where feasible; no prolonged unresponsive controls.
Set acceptance tolerances from physical measurements. If the goals fail:

1. Inspect sim vs draw time; reproduce the exact layout and round.
2. Reduce rendering cadence/detail or compare partial redraw if drawing dominates.
3. If simulation dominates, measure candidate checks and introduce buckets.
4. Tune deliberate scenario caps/waves; expose the limits instead of deleting
   enemies or silently skipping simulation steps.
5. Rebuild, rerun correctness tests, compare the same scenarios, then expand.

Do not reset/update the physical calculator to work around a game bug. Diagnose
the emulator case and code first. Calculator resets and file deletion require
separate user authorization.
