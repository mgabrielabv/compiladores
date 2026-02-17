@echo off
echo Eliminando archivos compilados...
del /q programa.exe 2>nul
del /q *.o 2>nul
echo Limpieza completada.
pause