[CmdletBinding()]
param (
    [string] $archiveName, [string] $targetName
)
# 外部环境变量包括:
# VCINSTALLDIR: 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC'
# archiveName: ${{ matrix.qt_ver }}-${{ matrix.qt_arch }}
# winSdkDir: ${{ steps.build.outputs.winSdkDir }}
# winSdkVer: ${{ steps.build.outputs.winSdkVer }}
# vcToolsInstallDir: ${{ steps.build.outputs.vcToolsInstallDir }}
# vcToolsRedistDir: ${{ steps.build.outputs.vcToolsRedistDir }}
# msvcArch: ${{ matrix.msvc_arch }}


# winSdkDir: C:\Program Files (x86)\Windows Kits\10\ 
# winSdkVer: 10.0.19041.0\ 
# vcToolsInstallDir: C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Tools\MSVC\14.28.29333\ 
# vcToolsRedistDir: C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Redist\MSVC\14.28.29325\ 
# VCINSTALLDIR: C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC
# archiveName: 5.9.9-win32_msvc2015
# msvcArch: x86

$scriptDir = $PSScriptRoot
$currentDir = Get-Location
Write-Host "currentDir" $currentDir
Write-Host "scriptDir" $scriptDir

function Main() {
    # 打包zip
    Compress-Archive -Path "release" $archiveName'.zip'
}

if ($null -eq $archiveName || $null -eq $targetName) {
    Write-Host "args missing, archiveName is" $archiveName ", targetName is" $targetName
    return
}
Main