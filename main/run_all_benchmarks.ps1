[CmdletBinding()]
param (
    [string]$TargetConfig = "",
    [ValidateSet("regular", "deep")]
    [string]$Mode = ""
)

if ($Mode) {
    $global:BENCHMARK_MODE = $Mode
}

$global:WORKSPACE_DIR = $PSScriptRoot
. "$PSScriptRoot\configs.ps1"
& "$PSScriptRoot\lib\base-lib\build\tools\run_benchmarks_impl.ps1" -TargetConfig $TargetConfig
exit $LASTEXITCODE
