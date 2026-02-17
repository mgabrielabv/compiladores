@echo off
echo ========================================
echo    COMPILANDO LIBRERIA ARRAYLIST
echo ========================================
echo.

echo 1. Verificando compilador...
g++ --version > nul 2>&1
if errorlevel 1 (
    echo ERROR: g++ no encontrado.
    echo Instala MinGW o agregalo al PATH.
    goto error
)

echo 2. Compilando main.cpp...
g++ -std=c++11 main.cpp -o programa.exe

if errorlevel 1 (
    echo ERROR: Fallo en compilacion.
    goto error
)

echo 3. Compilacion EXITOSA ✓
echo.
echo 4. Ejecutando programa...
echo ========================================
echo.
programa.exe

echo.
echo ========================================
echo Programa finalizado.
pause
exit /b 0

:error
echo.
echo ========================================
echo COMPILACION FALLIDA
pause
exit /b 1