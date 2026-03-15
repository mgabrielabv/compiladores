#include "semantico.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../analizadorlexico/lexer.h"

using namespace std;

int main()
{
    // Leer el código fuente Pascal desde el archivo code.pas
    ifstream file("../Analizador_sintactico/code.pas");
    if (!file)
    {
        cerr << "No se pudo abrir ../Analizador_sintactico/code.pas" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();

    // Inicializar palabras reservadas para Pascal
    Lexer::initReservedWords();

    // Tokenizar el código
    Lexer lexer(code);
    vector<Token> tokens;
    Token token;

    do
    {
        token = lexer.nextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::EndOfFile);

    // Analizar semánticamente
    SemanticAnalyzer analyzer(tokens);
    analyzer.analyze();

    // Mostrar solo los errores semánticos
    if (!analyzer.isSemanticallyCorrect())
    {
        cout << "Errores semánticos encontrados:\n";
        for (const auto &error : analyzer.getErrors())
        {
            cout << "Línea " << error.line << ", Columna " << error.column
                 << ": " << error.message << "\n";
        }
    }
    else
    {
        cout << "El código es semánticamente correcto.\n";
    }

    return 0;
}
