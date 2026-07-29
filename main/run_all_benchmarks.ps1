[CmdletBinding()]
param (
    [string]$TargetConfig = ""
)

$global:WORKSPACE_DIR = $PSScriptRoot
. "$PSScriptRoot\configs.ps1"
& "$PSScriptRoot\lib\base-lib\build\tools\run_benchmarks_impl.ps1" $TargetConfig
exit $LASTEXITCODE
