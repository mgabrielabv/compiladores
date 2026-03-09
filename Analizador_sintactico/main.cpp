#include "../analizadorlexico/lexer.h"        
#include "../Analizador_sintactico/parser.h"  
#include "../LibreriaDeSoportes/arraylist.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

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
    string nombreArchivo = "../analizadorlexico/code.pas"; 
    string codigoFuente = leerArchivo(nombreArchivo);

    cout << "Analizando archivo : " << nombreArchivo << "\n";
    cout << "-------------------------------------\n";
    cout << codigoFuente << endl;
    cout << "-------------------------------------\n";


    Lexer::initReservedWords(); 
    Lexer lexer(codigoFuente);
    ArrayList<Token> tokens;
    Token token;

    try {
        do
        {
            token = lexer.nextToken();
            tokens.add(token);
        } while (token.type != TokenType::EndOfFile);

        cout << "Lexer: Generados " << tokens.size() << " tokens con exito.\n\n";

      
        Parser parser(tokens);

   
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