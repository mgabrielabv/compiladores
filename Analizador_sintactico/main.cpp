#include "../analizadorlexico/lexer.h"        
#include "../Analizador_sintactico/parser.h"  
#include "../LibreriaDeSoportes/arraylist.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

string tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::Keyword: return "Keyword";
    case TokenType::Operator: return "Operator";
    case TokenType::Function: return "Function";
    case TokenType::Delimiter: return "Delimiter";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Number: return "Number";
    case TokenType::String: return "String";
    case TokenType::Unknown: return "Unknown";
    case TokenType::Type: return "Type";
    case TokenType::Boolean: return "Boolean";
    case TokenType::EndOfFile: return "EndOfFile";
    default: return "Invalid";
    }
}

string leerArchivo(const string &nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (!archivo) {
        cerr << "Error: No se pudo abrir el archivo fuente: " << nombreArchivo << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << archivo.rdbuf();
    return buffer.str();
}

int main() {
    Lexer::initReservedWords();

    string nombreArchivo = "code.pas";
    string codigoFuente = leerArchivo(nombreArchivo);

    cout << "Analizando archivo : " << nombreArchivo << "\n";
    cout << "-------------------------------------\n";
    cout << codigoFuente << endl;
    cout << "-------------------------------------\n";

    Lexer lexer(codigoFuente);
    ArrayList<Token> tokens;
    Token token;

    try {
        do {
            token = lexer.nextToken(); // Genera tokens hasta el final del archivo
            tokens.add(token);
        } while (token.type != TokenType::EndOfFile);

        cout << "Lexer: Generados " << tokens.size() << " tokens.\n\n";
        
        cout << "Lista de tokens:\n";
        for (size_t i = 0; i < tokens.size(); i++) { // Muestra cada token con su tipo y posicion
            Token t = tokens.get(i);
            cout << "  " << t.value << " [" << tokenTypeToString(t.type) 
                 << "] en linea " << t.line << "\n";
        }
        cout << "\n";

        Parser parser(tokens); // Crea el parser con la lista de tokens generada por el lexer

        cout << "Iniciando el Analisis Sintactico...\n";
        cout << "-------------------------------------\n";
        parser.parse();
        cout << "-------------------------------------\n";

        if (parser.hasSyntaxError) {
            cout << "\nEl analisis finalizo con errores de sintaxis.\n";
        } else {
            cout << "\nAnalisis completado exitosamente. No se encontraron errores.\n";
        }

    } catch (const exception &e) {
        cerr << "Error critico: " << e.what() << endl;
        return 1;
    }

    return 0;
}