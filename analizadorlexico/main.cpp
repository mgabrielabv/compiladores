#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

string tokenTypeToString(TokenType type) // convierte un tipo de token a un string
{
    switch (type) //evalua el valor del tipo 
    {
    case TokenType::Keyword:
        return "Keyword";
    case TokenType::Operator:
        return "Operator";
    case TokenType::Function:
        return "Function";
    case TokenType::Delimiter:
        return "Delimiter";
    case TokenType::Identifier:
        return "Identifier";
    case TokenType::Number:
        return "Number";
    case TokenType::String:
        return "String";
    // case TokenType::Comment:     return "Comment";
    case TokenType::Unknown:
        return "Unknown";
    case TokenType::EndOfFile:
        return "EndOfFile";
    default:
        return "Invalid";
    }
}

string readSourceFile(const string &filename)
{
    ifstream file(filename);
    if (!file)
    {
        cerr << "No se pudo abrir el archivo de codigo: " << filename << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main()
{
    Lexer::initReservedWords(); // Inicializa las palabras reservadas antes de crear el lexer

    string sourceCode = readSourceFile("code.pas");

    Lexer lexer(sourceCode);

    Token token; // Variable para almacenar el token actual
    do
    {
        token = lexer.nextToken(); // obtengo el siguiente token
        cout << "Token: '" << token.value << "' ("
             << tokenTypeToString(token.type) << ") at ["
             << token.line << "," << token.column << "]\n";
    } while (token.type != TokenType::EndOfFile);

    return 0;
}
