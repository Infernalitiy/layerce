param([string]$CEdev,[switch]$CoreTests)
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if (-not $CEdev -and (Test-Path '.tools\CEdev\bin')) {
    $CEdev = Join-Path (Get-Location) '.tools\CEdev'
}
if ($CEdev) {
    $toolRoot = (Resolve-Path -LiteralPath $CEdev).Path
    $env:PATH = "$toolRoot\bin;" + $env:PATH
}
if (-not (Get-Command cedev-config -ErrorAction SilentlyContinue)) {
    throw 'CEdev missing. Extract official v15.0 to a path without spaces and pass -CEdev C:\CEdev.'
}
if (-not (Get-Command make -ErrorAction SilentlyContinue)) { throw 'CEdev make missing from PATH.' }
$targetName = if ($CoreTests) { 'TESTCE' } else { 'LAYERCE' }
# Rebuild because test and game modes share object paths.
& make clean
if ($LASTEXITCODE -ne 0) { throw 'make clean failed' }
if ($CoreTests) {
    & make 'NAME=TESTCE' 'CFLAGS=-Wall -Wextra -Oz -std=c11 -DCORE_TEST' 'EXTRA_C_SOURCES=tests/core_tests.c' 'V=1' 2>&1 | Tee-Object build-test.log
} else {
    & make 'V=1' 2>&1 | Tee-Object build.log
}
if ($LASTEXITCODE -ne 0) { throw 'Build failed. Keep the log; do not transfer partial output.' }
$artifact = Join-Path (Get-Location) "bin\$targetName.8xp"
if (-not (Test-Path -LiteralPath $artifact)) { throw "Build did not produce $artifact" }
Get-Item -LiteralPath $artifact | Select-Object Name,Length,FullName
Get-FileHash -LiteralPath $artifact -Algorithm SHA256
Write-Host 'Inspect bin/*.map and run in CEmu before physical transfer.'
