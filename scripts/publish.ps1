param(
    [Parameter(Mandatory=$true)]
    $version,
    [Parameter(Mandatory=$true)]
    $library
)

function NuGetPackage {
    Invoke-WebRequest -Uri https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -OutFile "$BASE_DIR/nuget.exe"
    # ./nuget push "$srcDir/NugetPublish/Release/$library.$version.nupkg" 7a471658-677b-4d68-b782-83f2e1a56c70 -Source http://10.32.46.72:8081/nuget
    # ./nuget push "$srcDir/NugetPublish/Release/$library.$version.snupkg" 7a471658-677b-4d68-b782-83f2e1a56c70 -Source http://10.32.46.72:8081/nuget
    Remove-Item -Force "$srcDir/NugetPublish"
}

function Build {
    $srcDir = "$BASE_DIR/../src/$library"
    Set-Location $srcDir
    mkdir "$srcDir/NugetPublish"
    dotnet build "$srcDir/$library.csproj" --configuration Release --output "$srcDir/NugetPublish" -p:Version=$version
    # msbuild -p:Configuration=Release -p:Platform=x64 -p:OutDir="$srcDir/NugetPublish/Release"
}

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
if ([String]::IsNullOrEmpty($library)) {
    Write-Output "library should not be empty."
    exit
}

$BASE_DIR = $PSScriptRoot

Build
NuGetPackage

Set-Location $BASE_DIR