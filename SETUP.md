# Prepare now; develop the game later

The goal is a substantial original 2D tower defense adaptation for the TI-84
Plus CE Python. Later we will compile the existing prototype, test in CEmu,
measure the physical calculator, and expand upgrade choices. This setup session
does not build, transfer or run the game.

## What each tool does

| Tool | Purpose | Current state |
| --- | --- | --- |
| VS Code | Edit the project's source files | Installed; opening/control not confirmed by the assistant |
| Git | Local version history, commits and pushes | Installed; initial project commit/branches prepared locally |
| GitHub | Online copy of those commits | Target is infernalitiy/layerce; creation/authentication/first push still require completion |
| Python 3 | Small setup/backup scripts; no Python game runtime | Installed; no extra Python packages needed |
| CEdev v15.0 | Converts C source to a calculator `.8xp` | Installer task prepared; not yet downloaded/installed successfully |
| GraphX CE library | Drawing on the calculator | Present in prior screenshot; ABI version not confirmed |
| TI Connect CE | Transfers calculator programs/libraries | Already part of user's setup |
| CEmu + your ROM | Test before touching the physical calculator | User confirmed boot; game execution not yet tested |
| arTIfiCE / CEaShell | Launch native programs on the calculator | Both user-confirmed working; keep current installation |

No Node.js, npm packages, CMake, paid extensions or assembly tooling beyond CEdev
are required. The Microsoft C/C++ VS Code extension is optional for editing help.

## 1. Open the prepared workspace

Open VS Code yourself, then **File > Open Workspace from File** and select:

`C:\Users\inf\Documents\Codex\2026-11-02\new-chat-2\LAYERCE\LayerDefenseCE.code-workspace`

This is the actual project folder under Documents. Workspace auto-save is set to
one second after edits stop. Git commits snapshot saved disk files, not unsaved
editor buffers. The backup is an ordinary local VS Code task, not a cloud reminder
or a Windows startup service.

VS Code may ask whether you trust this workspace and whether to allow automatic
tasks. Review this project's scripts, then allow them only for this workspace
when ready. We have not changed system execution policy or bypassed these prompts.
If an automatic task initially reports that GitHub is unavailable, finish step 3
and restart that task.

## 2. Install the compiler without building the game

Use **Terminal > Run Task > Setup: install CE compiler**.
The script downloads the official v15.0 Windows ZIP and copies CEdev into
`.tools/CEdev`, inside this project. Downloads and tools are ignored by Git.
The installer prints the compiler version and local ZIP SHA-256; the hash is a
local receipt, not independent proof of authenticity. No game is built.

If downloading is blocked, get CEdev-Windows.zip yourself from the official
release and use the ordinary **VS Code PowerShell terminal**:

```powershell
powershell -NoProfile -File .\tools\install-dependencies.ps1 -ZipPath "$env:USERPROFILE\Downloads\CEdev-Windows.zip"
```

Official source: https://github.com/CE-Programming/toolchain/releases/tag/v15.0

An incomplete existing installation is left alone, and downloaded originals are
preserved. If the compiler needs a Microsoft runtime DLL, follow the official
CEdev setup page rather than changing calculator software or disabling security.

## 3. Connect GitHub once

This project is configured for **https://github.com/infernalitiy/layerce**.
The assistant could not verify whether it exists, create it, authenticate or
perform the first push in the restricted session.

1. Sign in to GitHub as **infernalitiy** in your normal browser.
2. Create a **private**, empty repository named **layerce**, if you do not already
   have one. Do not initialize it with README/license/gitignore: we have local
   files already. If you want another repository, change this configuration
   deliberately before starting backups.
3. In VS Code, select **Terminal > Run Task > Setup: connect GitHub repository**.
4. Complete any Git Credential Manager browser sign-in yourself. Never paste
   a token/password into these scripts, source files or chat.
5. The task pushes the initial `main` snapshot and then `dev`. It never force
   pushes or replaces a different `origin`. If an existing repository contains
   history, a rejected push is a reason to inspect/integrate it manually.

The connection task needs an initial local `main` commit and a `dev` branch.
If either is missing, make the first commit on main and create dev in VS Code
Source Control before retrying. Do not use a force push to resolve setup errors.

## 4. Enable the automatic backup task

Use **Terminal > Run Task > GitHub: automatic backup**. To start it on future
workspace opens, use **Tasks: Manage Automatic Tasks in Folder > Allow Automatic
Tasks** from the Command Palette, then reopen this workspace if required.

Its terminal should report `SYNC_READY`, followed by `Backed up...` or `Up to
date...`. `SYNC_READY` means the process started, **not** that GitHub is connected.
Verify a successful backup by checking the commit and file on GitHub's **dev**
branch. The default GitHub main view won't display new dev-only edits.

Behavior:

- Checks saved project changes every **120 seconds** while the task is running.
- Waits for files to be unchanged for at least 10 seconds before a checkpoint.
  Continuous editing may defer checkpoints until there is a quiet interval.
- Commits project code/config/docs and pushes only to **dev**. Main remains a
  stable checkpoint until we deliberately promote tested work later.
- Ignores ROMs, installed tools, logs, builds, `.env`, keys and unrelated files.
  Inline secrets in otherwise allowed source files are still your responsibility.
- Refuses a changed remote, another active branch, a merge/rebase in progress,
  or files you've manually staged. It does not merge, pull, reset or force-push.
- Network/auth errors leave your code and local commits intact. It retries on
  the next interval. A non-fast-forward rejection requires manual review; it
  will not overwrite the remote branch.
- Can back up incomplete or broken code; automatic backup is not a build pass.
- Runs only while the VS Code task is alive. Use **Terminal > Terminate Task**
  to stop it; stop it before manual branch operations or editing Git history.
  Closing VS Code normally stops its task, but confirm it terminates if the
  editor offers to keep terminals alive.

The workflow is **manual only**, and checkpoint commits also include `[skip ci]`.
Nothing compiles on each save. We will run correctness tests and builds explicitly
when development resumes. No paid GitHub Actions usage has been enabled.

## 5. Calculator dependencies can wait

Your existing arTIfiCE v2.1 and CEaShell launches work, so leave them alone.
Do not update/reset the calculator. Keep FONTLIBC even though this source doesn't
use it. Keep the unverified OS recovery bundle separate from this project.

A `clibs.8xg` file exists in Downloads, but its contents/version have not been
authenticated. Before transferring anything later, inventory your installed
libraries and compare the built game's loader requirements in CEmu. Then send
only the compatible dependencies actually needed. Archive space and runtime RAM
remain separate budgets.

## Current verification

The setup scripts' PowerShell syntax and workspace JSON are checked locally.
The automatic backup tests use temporary local repositories, not your live
GitHub account. See docs/RESULTS.md for exact pass/blocked counts. No successful
GitHub backup, installed compiler, game build, or emulator gameplay is claimed
until each is actually observed.

References:
- https://code.visualstudio.com/docs/sourcecontrol/overview
- https://code.visualstudio.com/docs/debugtest/tasks
- https://docs.github.com/en/actions/how-tos/manage-workflow-runs/skip-workflow-runs
