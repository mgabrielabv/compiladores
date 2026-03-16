#include "semantico.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../analizadorlexico/lexer.h"
#include "../Analizador_sintactico/parser.h"
#include "../LibreriaDeSoportes/arraylist.h"

using namespace std;

int main()
{
    // Leer el código fuente Pascal desde el archivo code.pas
    ifstream file("../code.pas");
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
    ArrayList<Token> tokens;
    Token token;

    do
    {
        token = lexer.nextToken();
        tokens.add(token);
    } while (token.type != TokenType::EndOfFile);

    // Analizar sintácticamente antes del semántico
    Parser parser(tokens);
    parser.parse();

    if (parser.hasSyntaxError)
    {
        cout << "No se ejecuta el analizador semántico porque hay errores sintácticos.\n";
        return 1;
    }

    // Analizar semánticamente
    SemanticAnalyzer analyzer(tokens);
    analyzer.analyze();

    // Mostrar solo los errores semánticos
    if (!analyzer.isSemanticallyCorrect())
    {
        cout << "Errores semánticos encontrados:\n";
        for (size_t i = 0; i < analyzer.getErrors().size(); ++i)
        {
            const auto &error = analyzer.getErrors().get(i);
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
