param([string]$Compiler='gcc')
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
if (-not (Get-Command $Compiler -ErrorAction SilentlyContinue)) { throw "Host C compiler missing: $Compiler" }
New-Item -ItemType Directory -Path test-bin -Force | Out-Null
& $Compiler -std=c11 -O2 -Wall -Wextra -Werror -Isrc src/game.c tests/core_tests.c -o test-bin/core-tests.exe
if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
& .\test-bin\core-tests.exe
if ($LASTEXITCODE -ne 0) { throw 'Core tests failed' }
