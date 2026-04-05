param(
    [ValidateSet("auto", "gcc", "clang", "cl")]
    [string]$Compiler = "auto",
    [ValidateRange(0, 1048576)]
    [int]$MemoryLimitKB = 16,
    [string]$ConfigPath = ""
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildScript = Join-Path $Root "scripts\build.ps1"
$BuildDir = Join-Path $Root "build"
$BenchDir = Join-Path $BuildDir "bench"
$Inc = Join-Path $Root "include"
$BenchSrc = Join-Path $Root "tools\osfx_bench_main.c"

function Resolve-Compiler([string]$Preferred) {
    if ($Preferred -ne "auto") { return $Preferred }
    if (Get-Command clang -ErrorAction SilentlyContinue) { return "clang" }
    if (Get-Command gcc -ErrorAction SilentlyContinue) { return "gcc" }
    if (Get-Command cl -ErrorAction SilentlyContinue) { return "cl" }
    throw "No C compiler found (clang/gcc/cl)."
}

New-Item -ItemType Directory -Force -Path $BenchDir | Out-Null
if (-not $ConfigPath) {
    $ConfigPath = Join-Path $Root "Config.json"
}

& powershell -ExecutionPolicy Bypass -File $BuildScript -Compiler $Compiler -ConfigPath $ConfigPath
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Selected = Resolve-Compiler $Compiler
Write-Host "[bench] compiler=$Selected"

if ($Selected -eq "clang" -or $Selected -eq "gcc") {
    $Lib = Join-Path $BuildDir "libosfx_core.a"
    $Exe = Join-Path $BuildDir "osfx_bench_main.exe"
    & $Selected -std=c99 -O3 -I $Inc $BenchSrc $Lib -lpsapi -o $Exe
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $Exe $BenchDir $MemoryLimitKB
    exit $LASTEXITCODE
}

if ($Selected -eq "cl") {
    $Obj = Join-Path $BuildDir "osfx_bench_main.obj"
    $Lib = Join-Path $BuildDir "osfx_core.lib"
    $Exe = Join-Path $BuildDir "osfx_bench_main.exe"
    & cl /nologo /TC /O2 /D_CRT_SECURE_NO_WARNINGS /I $Inc /c $BenchSrc /Fo$Obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & link /nologo /OUT:$Exe $Obj $Lib Psapi.lib
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $Exe $BenchDir $MemoryLimitKB
    exit $LASTEXITCODE
}

throw "Unsupported compiler: $Selected"

