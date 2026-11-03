param([string]$Repository='infernalitiy/layerce')
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if ($Repository -ne 'infernalitiy/layerce') { throw 'Review tools/sync-config.json before choosing a different repository.' }
$remoteUrl = "https://github.com/$Repository.git"
Write-Host "Use an EMPTY PRIVATE repository at $remoteUrl. Create it in GitHub first."
Write-Host 'Your existing Git Credential Manager may open GitHub sign-in; complete it yourself.'
$remoteNames = @(& git remote)
if ($remoteNames -contains 'origin') {
    $existing = & git remote get-url origin
    if ($existing -ne $remoteUrl) { throw 'origin already points somewhere else. Inspect it; no remote was changed.' }
} else {
    & git remote add origin $remoteUrl
    if ($LASTEXITCODE -ne 0) { throw 'Could not set remote' }
}
& git show-ref --verify --quiet refs/heads/main
if ($LASTEXITCODE -ne 0) { throw 'Initial main commit is missing. Commit the prepared files before connecting.' }
& git push -u origin main
if ($LASTEXITCODE -ne 0) { throw 'Initial push failed. Check sign-in, repository existence, and network. No history was overwritten.' }
$branch = (& git branch --show-current).Trim()
if ($branch -ne 'dev') {
    & git switch dev
    if ($LASTEXITCODE -ne 0) { throw 'Switch to the prepared dev branch manually.' }
}
& git push -u origin dev
if ($LASTEXITCODE -ne 0) { throw 'dev push failed; local files and commits retained.' }
Write-Host 'Connected. Start the VS Code task GitHub: automatic backup, or reopen and allow its folder task.'
