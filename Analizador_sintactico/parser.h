#ifndef PARSER_H
#define PARSER_H

#include "../analizadorlexico/lexer.h"
#include "../LibreriaDeSoportes/arraylist.h"
#include <string>
#include <sstream>

using namespace std;

class ExprNode; // ya las clases estan definidas
class ExpressionParser;

struct SyntaxError {
    string message;
    int line;
    int column;
    string token;
    string context;
};

class Parser
{
public:
    Parser(const ArrayList<Token> &tokens);
    void parse(); // metodo que inicia el analisis sintactico
    bool hasSyntaxError = false;
    ArrayList<SyntaxError> getErrors() const { return errors; } // devuelve los errores encontrados 

private:
    const ArrayList<Token> &tokens; // lista de tokens para analizar
    size_t pos = 0;
    ArrayList<SyntaxError> errors; // lista de errores encontrados durante el analisis
    string currentContext; // parte del codigo 

    Token peek();
    Token get();
    bool match(TokenType type);
    bool match(const string &value);
    bool isAtEnd();
    void error(const string& message);
    void setContext(const string& context);
    
    void program();
    void block(); // maneja bloques de codigo 
    void declarations(); // se analizan las variables declaradas
    void statement(); //para instrucciones tipo if, while, etc
    void assignmentOrCall(); // para saber si es una asignacion o funcion
    
    Token expectIdentifier();
    void expect(const string &value);
    void expectSemicolon();
    void expectEnd();
    void recoverTo(const string& delimiter);
};

#endif