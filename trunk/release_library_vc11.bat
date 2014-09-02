set VCVER=vc11
if not exist Distribute mkdir .\Distribute
if not exist Distribute\Include mkdir .\Distribute\Include
if not exist Distribute\Lib mkdir .\Distribute\Lib
if not exist Distribute\Lib\%VCVER% mkdir .\Distribute\Lib\%VCVER%
if not exist Distribute\Lib\%VCVER%\x86 mkdir .\Distribute\Lib\%VCVER%\x86
if not exist Distribute\Lib\%VCVER%\x64 mkdir .\Distribute\Lib\%VCVER%\x64
if not exist Distribute\Bin mkdir .\Distribute\Bin
if not exist Distribute\Bin\%VCVER% mkdir .\Distribute\Bin\%VCVER%
if not exist Distribute\Bin\%VCVER%\x86 mkdir .\Distribute\Bin\%VCVER%\x86
if not exist Distribute\Bin\%VCVER%\x64 mkdir .\Distribute\Bin\%VCVER%\x64

xcopy /y/s XUtil\Include Distribute\Include
xcopy /y/s XImage\Include Distribute\Include
xcopy /y/s XGraphic\Include Distribute\Include
xcopy /y/s XImageView\Include Distribute\Include

copy /y XUtil\Lib\%VCVER%\x86\*.lib Distribute\Lib\%VCVER%\x86\
copy /y XImage\Lib\%VCVER%\x86\*.lib Distribute\Lib\%VCVER%\x86\
copy /y XGraphic\Lib\%VCVER%\x86\*.lib Distribute\Lib\%VCVER%\x86\
copy /y XImageView\Lib\%VCVER%\x86\*.lib Distribute\Lib\%VCVER%\x86\

copy /y XUtil\Lib\%VCVER%\x64\*.lib Distribute\Lib\%VCVER%\x64\
copy /y XImage\Lib\%VCVER%\x64\*.lib Distribute\Lib\%VCVER%\x64\
copy /y XGraphic\Lib\%VCVER%\x64\*.lib Distribute\Lib\%VCVER%\x64\
copy /y XImageView\Lib\%VCVER%\x64\*.lib Distribute\Lib\%VCVER%\x64\

copy /y XUtil\Bin\%VCVER%\x86\*.dll Distribute\Bin\%VCVER%\x86\
copy /y XImage\Bin\%VCVER%\x86\*.dll Distribute\Bin\%VCVER%\x86\
copy /y XGraphic\Bin\%VCVER%\x86\*.dll Distribute\Bin\%VCVER%\x86\
copy /y XImageView\Bin\%VCVER%\x86\*.dll Distribute\Bin\%VCVER%\x86\

copy /y XUtil\Bin\%VCVER%\x64\*.dll Distribute\Bin\%VCVER%\x64\
copy /y XImage\Bin\%VCVER%\x64\*.dll Distribute\Bin\%VCVER%\x64\
copy /y XGraphic\Bin\%VCVER%\x64\*.dll Distribute\Bin\%VCVER%\x64\
copy /y XImageView\Bin\%VCVER%\x64\*.dll Distribute\Bin\%VCVER%\x64\

copy /y COPYING Distribute\Bin\%VCVER%\x86\
copy /y COPYING Distribute\Bin\%VCVER%\x64\