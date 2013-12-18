@echo off
set BUILD_OPT=Rebuild
echo Begin: Build for vc11 (x86)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
if exist "C:\Program Files\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" call "C:\Program Files\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
echo Unicode Debug building...
msbuild /p:Configuration=Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Unicode Release building...
msbuild /p:Configuration=Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Multibyte Debug building...
msbuild /p:Configuration=Multibyte_Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Multibyte Release building...
msbuild /p:Configuration=Multibyte_Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Finish: Build for vc11 (x86)

echo Begin: Build for vc11 (x64)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
if exist "C:\Program Files\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" call "C:\Program Files\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
echo Unicode Debug building...
msbuild /p:Configuration=Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Unicode Release building...
msbuild /p:Configuration=Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Multibyte Debug building...
msbuild /p:Configuration=Multibyte_Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Multibyte Release building...
msbuild /p:Configuration=Multibyte_Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc11.sln
echo Finish: Build for vc11 (x64)

echo copy distribute
call release_library_vc11.bat

echo Begin: Build for vc10 (x86)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
if exist "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
echo Unicode Debug building...
msbuild /p:Configuration=Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Unicode Release building...
msbuild /p:Configuration=Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Multibyte Debug building...
msbuild /p:Configuration=Multibyte_Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Multibyte Release building...
msbuild /p:Configuration=Multibyte_Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Finish: Build for vc10 (x86)

echo Begin: Build for vc10 (x64)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
if exist "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
echo Unicode Debug building...
msbuild /p:Configuration=Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Unicode Release building...
msbuild /p:Configuration=Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Multibyte Debug building...
msbuild /p:Configuration=Multibyte_Debug /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Multibyte Release building...
msbuild /p:Configuration=Multibyte_Release /t:%BUILD_OPT% /clp:ErrorsOnly /m Libraries.vc10.sln
echo Finish: Build for vc10 (x64)

echo copy distribute
call release_library_vc10.bat