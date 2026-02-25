#include "../analizadorlexico/lexer.h"        
#include "../Analizador_sintactico/parser.h"  
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// Funcion para leer archivo fuente
string leerArchivo(const string &nombreArchivo)
{
    ifstream archivo(nombreArchivo);
    if (!archivo)
    {
        cerr << "Error: No se pudo abrir el archivo fuente: " << nombreArchivo << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << archivo.rdbuf();
    return buffer.str();
}

int main()
{
    string nombreArchivo = "./analizadorlexico/code.pas"; 
    string codigoFuente = leerArchivo(nombreArchivo);

    cout << "Analizando archivo : " << nombreArchivo << "\n";
    cout << "-------------------------------------\n";
    cout << codigoFuente << endl;
    cout << "-------------------------------------\n";

    // 1. Fase de Analisis Lexico
    Lexer lexer(codigoFuente);
    vector<Token> tokens;
    Token token;

    try {
        do
        {
            token = lexer.nextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::EndOfFile);

        cout << "Lexer: Generados " << tokens.size() << " tokens con exito.\n\n";

        // 2. Fase de Analisis Sintactico (Parser)
        Parser parser(tokens);
        
        parser.loadOperators("tokens.txt"); 
        
        cout << "Iniciando el Analisis Sintactico...\n";
        cout << "-------------------------------------\n";
        parser.parse();
        cout << "-------------------------------------\n";

        if (parser.hasSyntaxError) {
            cout << "\nEl analisis finalizo con errores de sintaxis.\n";
        } else {
            cout << "\nAnalisis completado exitosamente.\n";
        }

    } catch (const exception &e) {
        cerr << "Error critico durante la ejecucion: " << e.what() << endl;
        return 1;
    }

    return 0;
}