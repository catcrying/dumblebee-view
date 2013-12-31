@echo off

echo clean up
call cleanup.bat

echo build all configuration...

call build_vc10_x86.bat
call build_vc10_x64.bat
call build_vc11_x86.bat
call build_vc11_x64.bat

echo copy distribute
call release_library_vc10.bat
call release_library_vc11.bat

echo finish...