param(
    [Parameter(Mandatory=$true)]
    $version,
    [Parameter(Mandatory=$true)]
    $nugetSourcePath
)

if ([String]::IsNullOrEmpty($version)) {
    Write-Output "version should not be empty."
    exit
}
else {
    $result = $version -match '\d\.\d\.\d'
    if ($false -eq $result) {
        Write-Output "version format error."
        exit
    }
}
if ([String]::IsNullOrEmpty($nugetSourcePath)) {
    Write-Output "nugetSourcePath should not be empty."
    exit
}

function NuGetPackage {
    # Invoke-WebRequest -Uri https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -OutFile "$BASE_DIR/nugetPack/nuget.exe"
    Set-Location "$BASE_DIR/nugetPack"
    ./nuget pack -Version $version -Verbosity detailed
    Copy-Item -Path "$BASE_DIR/nugetPack/*" -Filter "*.nupkg" -Destination "$nugetSourcePath"
    ./nuget push "$BASE_DIR/nugetPack/Render.Interop.$version.nupkg" 7a471658-677b-4d68-b782-83f2e1a56c70 -Source http://10.32.46.72:8081/nuget
    Remove-Item -Force -Path "$BASE_DIR/nugetPack/*" -Filter "*.nupkg"
    # Remove-Item -Force "$BASE_DIR/nugetPack/nuget.exe"
}

function Build {
    $srcDir = "$BASE_DIR/../.."
    $libyuvOutputDir = "$BASE_DIR/../../libs"
    Set-Location "$srcDir/libyuv"
    # mkdir "$($libyuvOutputDir)"
    mkdir -Force "$libyuvOutputDir/Release"
    msbuild -p:Configuration=Release -p:Platform=x64 -p:OutDir="$libyuvOutputDir/Release"
    Set-Location "$BASE_DIR/../"
    msbuild -p:Configuration=Release -p:Platform=x64 -p:OutDir="$BASE_DIR/nugetPack/lib/netcoreapp3.1"
}

$BASE_DIR = $PSScriptRoot

Build
NuGetPackage

Set-Location $BASE_DIR