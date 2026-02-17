@echo off
pushd "%~dp0"
echo Compilando Analizador de Expresiones...
echo Directorio del script: %~dp0

REM Compilar incluyendo las rutas de los otros módulos
g++ -std=c++11 -o analizador_expresiones.exe ^
    main.cpp ^
    expression_parser.cpp ^
    ../analizadorlexico/lexer.cpp ^
    -I. -I../analizadorlexico -I../LibreriaDeSoportes 

if %errorlevel% equ 0 (
    echo.
    echo Compilacion exitosa!
    echo Ejecuta: analizador_expresiones.exe
) else (
    echo.
    echo Error en compilacion
)
popd
pause