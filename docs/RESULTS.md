# Validation record — initial snapshot

## Setup follow-up — November 3, 2026

- Feature development paused at the user's request; no game build/run attempted.
- Added project-local CEdev installer, VS Code auto-save and folder-open backup
  task, remote connection task, and a beginner setup guide.
- Automatic-backup tests: **5 passed, 2 skipped**. Passing checks exercise real
  local commits, preservation after a simulated network error, dry-run behavior,
  manual-staging refusal, branch/remote checks, quiet interval and session locking.
  Two end-to-end local-push tests are blocked by this execution environment:
  Git for Windows' sh.exe cannot create its required Windows named object.
  These are not successful GitHub transport tests.
- PowerShell syntax checks passed for all four setup/build scripts; workspace
  and task JSON parsed successfully. Source data checks still pass.
- GitHub HTTPS download remains blocked by host DNS resolution. CEdev installation
  is prepared but pending. No claim that the compiler has been installed.
- Desktop-control helper failed to launch; VS Code opening and prompts remain
  user steps. GitHub repository creation, sign-in and first push remain pending.
- GitHub Actions now runs only when manually dispatched, honoring the request
  to prepare now and build later. Automatic backup is not active until the
  task runs, GitHub access is working, and a successful push is observed.

## Performed during authoring

- Local source files and VS Code task configuration created.
- Existing setup: user confirmed physical arTIfiCE and CEaShell launches and
  successful CEmu boot. No independent observation of these launches.
- Python source table/arithmetic checks: PASS after correcting an initially
  incorrect expected route length. Correct values: 520 px, 8 waves, 13 groups,
  258 bloons, 766 total layers. The checker reads actual C source tables.
- Manual source review corrected drawing of the upward path segment.
- No CEdev or host C compiler was found on PATH or in the inspected development
  locations. Shell download failed DNS resolution; official compiler browser
  download timed out. No compiler was installed or run.
- VS Code opening was attempted through the CLI and direct executable; no
  successful editor launch was observed in the restricted workspace session.

## Still unmeasured/unverified

| Check | Result |
| --- | --- |
| CEdev compilation / warnings | Pending |
| Host C engine tests / sanitizers | Pending |
| TESTCE zero-failure run in CEmu | Pending |
| LAYERCE gameplay in CEmu | Pending |
| Physical gameplay / clean exits | Pending |
| FPS / SPS / input latency | Pending |
| Program variable / transfer file size | Pending |
| Target sizeof / actual BSS / stack / heap reserve | Pending |
| Loader dependency versions and exact total installed size | Pending |
| GitHub Actions workflow result | Pending; workflow configured only |

## Measurement template

Copy this table for each compiled commit. Record real values; leave unknowns blank.

| Field | Value |
| --- | --- |
| Commit, binary SHA-256, CEdev version | |
| Device/model/OS or CEmu version + 100% throttle | |
| Scenario: rounds/layout or 30s stress; speed | |
| FPS / SPS (three runs, typical and lowest observed) | |
| SIM average / max clock ticks + clock Hz | |
| DRAW average / max clock ticks | |
| Game/Bloon/Dart/Tower sizeof | |
| Bloon/dart peaks, slow events, pool waits | |
| OS user free RAM at launch (not heap) | |
| `.8xp` file bytes / calculator variable archive bytes | |
| Loaded code/BSS/heap boundaries + library residency | |
| Stack high-water method and observed maximum | |
| Dependency/launcher installed bytes and incremental delta | |
| Life/money/accounting correctness, clean exit | |
| Issues/reproduction steps | |
