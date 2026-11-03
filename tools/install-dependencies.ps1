param([string]$ZipPath)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$toolsRoot = Join-Path $projectRoot '.tools'
$installRoot = Join-Path $toolsRoot 'CEdev'
if (Test-Path -LiteralPath (Join-Path $installRoot 'bin\cedev-config.exe')) {
    Write-Host "CEdev is already installed: $installRoot"
    exit 0
}
if (Test-Path -LiteralPath $installRoot) {
    throw 'An incomplete CEdev folder exists. Inspect it manually; this installer will not delete or overwrite it.'
}
New-Item -ItemType Directory -Path $toolsRoot -Force | Out-Null
if (-not $ZipPath) {
    $ZipPath = Join-Path $toolsRoot 'CEdev-Windows-v15.0.zip'
    if (-not (Test-Path -LiteralPath $ZipPath)) {
        Write-Host 'Downloading pinned official CEdev v15.0 (Windows)...'
        Invoke-WebRequest -Uri 'https://github.com/CE-Programming/toolchain/releases/download/v15.0/CEdev-Windows.zip' -OutFile $ZipPath
    }
}
$ZipPath = (Resolve-Path -LiteralPath $ZipPath).Path
$extractRoot = Join-Path $toolsRoot ('extract-' + [Guid]::NewGuid().ToString('N'))
Expand-Archive -LiteralPath $ZipPath -DestinationPath $extractRoot
$candidates = @(Get-ChildItem -LiteralPath $extractRoot -Recurse -File -Filter 'cedev-config.exe' | Where-Object { $_.Directory.Name -eq 'bin' })
if ($candidates.Count -ne 1) { throw 'Expected exactly one bin/cedev-config.exe in the official archive.' }
$sourceRoot = $candidates[0].Directory.Parent.FullName
# Copy only into this new ignored folder; preserve all downloaded/extracted originals.
Copy-Item -LiteralPath $sourceRoot -Destination $installRoot -Recurse
if ($installRoot -match '\s') { throw 'CEdev needs a path without spaces. Move the project manually to a suitable path before building.' }
Get-FileHash -LiteralPath $ZipPath -Algorithm SHA256 | Format-List
$compiler = Join-Path $installRoot 'bin\ez80-clang.exe'
if (-not (Test-Path -LiteralPath $compiler)) { throw 'Compiler executable missing; inspect the extracted package.' }
& $compiler --version
if ($LASTEXITCODE -ne 0) { throw 'Compiler did not start. Check official CEdev DLL requirements.' }
Write-Host "Installed compiler in $installRoot"
Write-Host 'No game was built or transferred. No calculator libraries, launchers, or OS were changed.'
