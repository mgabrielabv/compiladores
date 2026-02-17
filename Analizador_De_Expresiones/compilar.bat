@echo off
REM Compila el Analizador de Expresiones en Windows usando g++

setlocal


set SRC=fuente\main.cpp fuente\expression_parser.cpp ..\LibreriaDeSoportes\arraylist.h "C:\Users\gaby's\OneDrive\Documents\Compiladores\analizadorLexico\lexer.cpp"
set INC=-Iincluye -I..\LibreriaDeSoportes -I..\..\..\analizadorLexico
set OUT=analizador_expresiones.exe

g++ %INC% %SRC% -o %OUT% -std=c++17 -Wall

if %ERRORLEVEL%==0 (
    echo Compilacion exitosa: %OUT%
) else (
    echo Error en la compilacion
)
endlocal
pause
