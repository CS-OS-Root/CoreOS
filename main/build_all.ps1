[CmdletBinding()]
param (
    [string]$TargetConfig = ""
)

$global:WORKSPACE_DIR = $PSScriptRoot
. "$PSScriptRoot\configs.ps1"
& "$PSScriptRoot\lib\base-lib\build\tools\build_all_impl.ps1" $TargetConfig
exit $LASTEXITCODE
