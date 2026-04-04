param(
    [ValidateSet("auto", "gcc", "clang", "cl")]
    [string]$Compiler = "auto",
    [switch]$Matrix,
    [string]$ConfigPath = ""
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildScript = Join-Path $Root "scripts\build.ps1"
$BuildDir = Join-Path $Root "build"
$Inc = Join-Path $Root "include"
$TestSrc = Join-Path $Root "tests\test_osfx_core.c"
$IntSrc = Join-Path $Root "tests\test_osfx_integration.c"
$CliSrc = Join-Path $Root "tools\osfx_cli_main.c"
$ReportPath = Join-Path $BuildDir "quality_gate_report.md"
$ReportRows = @()

if (-not $ConfigPath) {
    $ConfigPath = Join-Path $Root "Config.json"
}

& powershell -ExecutionPolicy Bypass -File $BuildScript -Compiler $Compiler -ConfigPath $ConfigPath
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

function Resolve-Compiler([string]$Preferred) {
    if ($Preferred -ne "auto") { return $Preferred }
    if (Get-Command clang -ErrorAction SilentlyContinue) { return "clang" }
    if (Get-Command gcc -ErrorAction SilentlyContinue) { return "gcc" }
    if (Get-Command cl -ErrorAction SilentlyContinue) { return "cl" }
    throw "No C compiler found (clang/gcc/cl)."
}

function Test-CompilerAvailable([string]$Name) {
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Start-Report {
    $script:ReportRows = @()
}

function Add-ReportRow([string]$Name, [string]$Native, [string]$Integration, [string]$CliSmoke, [string]$Status) {
    $script:ReportRows += "| $Name | $Native | $Integration | $CliSmoke | $Status |"
}

function Write-Report {
    "# osfx-c99 quality gate report" | Set-Content -Path $ReportPath
    "" | Add-Content -Path $ReportPath
    "Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")" | Add-Content -Path $ReportPath
    "" | Add-Content -Path $ReportPath
    "| Compiler | Native | Integration | CLI Smoke | Status |" | Add-Content -Path $ReportPath
    "|---|---|---|---|---|" | Add-Content -Path $ReportPath
    foreach ($row in $ReportRows) {
        $row | Add-Content -Path $ReportPath
    }
}

function Invoke-CliSmoke([string]$CliExe) {
    $out = & $CliExe plugin-list 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0 -or $out -notmatch "transport" -or $out -notmatch "test_plugin" -or $out -notmatch "port_forwarder") {
        throw "CLI smoke failed: plugin-list"
    }
    & $CliExe plugin-load transport | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "CLI smoke failed: plugin-load transport" }
    & $CliExe plugin-load test_plugin | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "CLI smoke failed: plugin-load test_plugin" }
    & $CliExe plugin-load port_forwarder | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "CLI smoke failed: plugin-load port_forwarder" }

    $out = & $CliExe transport-status 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0 -or $out -notmatch "transport initialized=1") {
        throw "CLI smoke failed: transport-status"
    }
    $out = & $CliExe test-plugin run component 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0 -or $out -notmatch "ok=1") {
        throw "CLI smoke failed: test-plugin run"
    }
}

function Invoke-TestForCompiler([string]$Selected) {
    Write-Host "[test] compiler=$Selected"

    & powershell -ExecutionPolicy Bypass -File $BuildScript -Compiler $Selected -ConfigPath $ConfigPath
    if ($LASTEXITCODE -ne 0) { throw "build failed for $Selected" }

    if ($Selected -eq "gcc" -or $Selected -eq "clang") {
        $Exe = Join-Path $BuildDir ("test_osfx_core_" + $Selected + ".exe")
        $IntExe = Join-Path $BuildDir ("test_osfx_integration_" + $Selected + ".exe")
        $CliExe = Join-Path $BuildDir ("osfx_cli_" + $Selected + ".exe")
        $Lib = Join-Path $BuildDir "libosfx_core.a"

        & $Selected -std=c99 -O2 -D_CRT_SECURE_NO_WARNINGS -I $Inc $TestSrc $Lib -o $Exe
        if ($LASTEXITCODE -ne 0) { throw "native compile failed for $Selected" }
        & $Exe
        if ($LASTEXITCODE -ne 0) { throw "native test failed for $Selected" }

        & $Selected -std=c99 -O2 -D_CRT_SECURE_NO_WARNINGS -I $Inc $IntSrc $Lib -o $IntExe
        if ($LASTEXITCODE -ne 0) { throw "integration compile failed for $Selected" }
        & $IntExe
        if ($LASTEXITCODE -ne 0) { throw "integration test failed for $Selected" }

        & $Selected -std=c99 -O2 -D_CRT_SECURE_NO_WARNINGS -I $Inc $CliSrc $Lib -o $CliExe
        if ($LASTEXITCODE -ne 0) { throw "cli compile failed for $Selected" }
        Invoke-CliSmoke $CliExe
        return
    }

    if ($Selected -eq "cl") {
        $Exe = Join-Path $BuildDir "test_osfx_core_cl.exe"
        $IntExe = Join-Path $BuildDir "test_osfx_integration_cl.exe"
        $CliExe = Join-Path $BuildDir "osfx_cli_cl.exe"
        $Obj = Join-Path $BuildDir "test_osfx_core.obj"
        $IntObj = Join-Path $BuildDir "test_osfx_integration.obj"
        $CliObj = Join-Path $BuildDir "osfx_cli_main.obj"
        $Lib = Join-Path $BuildDir "osfx_core.lib"

        & cl /nologo /TC /O2 /D_CRT_SECURE_NO_WARNINGS /I $Inc /c $TestSrc /Fo$Obj
        if ($LASTEXITCODE -ne 0) { throw "native compile failed for cl" }
        & link /nologo /OUT:$Exe $Obj $Lib
        if ($LASTEXITCODE -ne 0) { throw "native link failed for cl" }
        & $Exe
        if ($LASTEXITCODE -ne 0) { throw "native test failed for cl" }

        & cl /nologo /TC /O2 /D_CRT_SECURE_NO_WARNINGS /I $Inc /c $IntSrc /Fo$IntObj
        if ($LASTEXITCODE -ne 0) { throw "integration compile failed for cl" }
        & link /nologo /OUT:$IntExe $IntObj $Lib
        if ($LASTEXITCODE -ne 0) { throw "integration link failed for cl" }
        & $IntExe
        if ($LASTEXITCODE -ne 0) { throw "integration test failed for cl" }

        & cl /nologo /TC /O2 /D_CRT_SECURE_NO_WARNINGS /I $Inc /c $CliSrc /Fo$CliObj
        if ($LASTEXITCODE -ne 0) { throw "cli compile failed for cl" }
        & link /nologo /OUT:$CliExe $CliObj $Lib
        if ($LASTEXITCODE -ne 0) { throw "cli link failed for cl" }
        Invoke-CliSmoke $CliExe
        return
    }

    throw "Unsupported compiler: $Selected"
}

Start-Report

if ($Matrix) {
    $targets = @("clang", "gcc", "cl")
    $hadFailure = $false
    foreach ($c in $targets) {
        if (-not (Test-CompilerAvailable $c)) {
            Write-Host "[skip] compiler=$c (not available)"
            Add-ReportRow $c "SKIP" "SKIP" "SKIP" "SKIP"
            continue
        }
        try {
            Invoke-TestForCompiler $c
            Add-ReportRow $c "PASS" "PASS" "PASS" "PASS"
        } catch {
            $hadFailure = $true
            Write-Host "[fail] compiler=${c}: $($_.Exception.Message)"
            Add-ReportRow $c "FAIL" "FAIL" "FAIL" "FAIL"
        }
    }
    if ($hadFailure) {
        Write-Report
        throw "quality gate failed; see $ReportPath"
    }
    Write-Report
    Write-Host "[ok] quality gate report: $ReportPath"
    exit 0
}

$Selected = Resolve-Compiler $Compiler
try {
    Invoke-TestForCompiler $Selected
    Add-ReportRow $Selected "PASS" "PASS" "PASS" "PASS"
    Write-Report
    Write-Host "[ok] quality gate report: $ReportPath"
    exit 0
} catch {
    Add-ReportRow $Selected "FAIL" "FAIL" "FAIL" "FAIL"
    Write-Report
    throw
}

