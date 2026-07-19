SET GCLI_PATH=..\..\..\gcli
SET PATH=%GCLI_PATH%
SET CLIENT=oilgas6srv-win.tar
tar -cvf %CLIENT% gctask oilgas6solve.exe
rem tar -rvf %CLIENT% FEC/Scite/tools/strpack.dll
gzip -f9 %CLIENT%
move %CLIENT%.gz ..\