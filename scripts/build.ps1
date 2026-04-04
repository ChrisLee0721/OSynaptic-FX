param(
    [ValidateSet("auto", "gcc", "clang", "cl")]
    [string]$Compiler = "auto",
    [string]$ConfigPath = ""
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$SrcDir = Join-Path $Root "src"
$Inc = Join-Path $Root "include"
$BuildDir = Join-Path $Root "build"
$BuildConfigHeader = Join-Path $Inc "osfx_build_config.h"
$LibraryDataHeader = Join-Path $SrcDir "osfx_library_data.generated.h"
if (-not $ConfigPath) {
    $ConfigPath = Join-Path $Root "Config.json"
}

New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
Get-ChildItem -Path $BuildDir -File -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue

if (-not (Test-Path $LibraryDataHeader)) {
    throw "Missing required generated header: $LibraryDataHeader"
}
if (-not (Test-Path $BuildConfigHeader)) {
    throw "Missing required build config header: $BuildConfigHeader"
}

function Resolve-Compiler([string]$Preferred) {
    if ($Preferred -ne "auto") {
        return $Preferred
    }
    if (Get-Command clang -ErrorAction SilentlyContinue) { return "clang" }
    if (Get-Command gcc -ErrorAction SilentlyContinue) { return "gcc" }
    if (Get-Command cl -ErrorAction SilentlyContinue) { return "cl" }
    throw "No C compiler found (clang/gcc/cl)."
}

$Selected = Resolve-Compiler $Compiler
Write-Host "[build] compiler=$Selected"

if ($Selected -eq "gcc" -or $Selected -eq "clang") {
    $ObjFiles = @()
    $Lib = Join-Path $BuildDir "libosfx_core.a"

    Get-ChildItem -Path $SrcDir -Filter "*.c" | ForEach-Object {
        $Obj = Join-Path $BuildDir ($_.BaseName + ".o")
        & $Selected -std=c99 -O3 -D_CRT_SECURE_NO_WARNINGS -I $Inc -c $_.FullName -o $Obj
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        $ObjFiles += $Obj
    }

    $Ar = Get-Command ar -ErrorAction SilentlyContinue
    if (-not $Ar) { throw "ar not found for static archive creation." }

    & $Ar.Source rcs $Lib $ObjFiles
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Write-Host "[ok] built $Lib"
    exit 0
}

if ($Selected -eq "cl") {
    $ObjFiles = @()
    $Lib = Join-Path $BuildDir "osfx_core.lib"

    Get-ChildItem -Path $SrcDir -Filter "*.c" | ForEach-Object {
        $Obj = Join-Path $BuildDir ($_.BaseName + ".obj")
        & cl /nologo /TC /O2 /D_CRT_SECURE_NO_WARNINGS /I $Inc /c $_.FullName /Fo$Obj
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        $ObjFiles += $Obj
    }

    $LibExe = Get-Command lib -ErrorAction SilentlyContinue
    if (-not $LibExe) { throw "lib.exe not found for static archive creation." }

    & $LibExe.Source /nologo /OUT:$Lib $ObjFiles
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Write-Host "[ok] built $Lib"
    exit 0
}

throw "Unsupported compiler: $Selected"

