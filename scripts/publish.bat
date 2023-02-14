call "%vcvarsPath%/vcvars64.bat"
set BASE_DIR=%~dp0..\src
set OUTPUT_DIR=%BASE_DIR%\..\output

msbuild.exe "%BASE_DIR%\RCWpfRender.sln" -t:Clean -p:Configuration=Release -p:Platform=x64
dotnet restore "%BASE_DIR%\RCWpfRender.sln"  -v n
cd "%BASE_DIR%\Render.Interop"
msbuild -t:restore

cd "%BASE_DIR%\libyuv"
mkdir %BASE_DIR%\libs
mkdir %BASE_DIR%\libs\Release
msbuild -p:Configuration=Release -p:Platform=x64 -p:OutDir="%BASE_DIR%\libs\Release"

cd "%BASE_DIR%\SampleApp"
msbuild -p:Configuration=Release -p:Platform=x64
@REM msbuild -t:Publish -p:SelfContained=True -p:PublishSingleFile=False -p:PublishProtocol=FileSystem -p:Configuration=Release -p:Platform=x64 -p:TargetFrameworks=netcoreapp3.1 -p:PublishDir="%OUTPUT_DIR%" -p:RuntimeIdentifier="win-x64" -p:PublishReadyToRun=False -p:PublishTrimmed=False

dotnet publish "%BASE_DIR%\SampleApp\SampleApp.csproj" -o "%OUTPUT_DIR%" --self-contained -f netcoreapp3.1 -r win-x64 -c Release --no-build --no-restore