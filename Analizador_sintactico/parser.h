#ifndef PARSER_H
#define PARSER_H

#include "../analizadorlexico/lexer.h"
#include "../LibreriaDeSoportes/arraylist.h"
#include <string>
#include <sstream>

using namespace std;

class ExprNode;
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
    void parse();
    bool hasSyntaxError = false;
    ArrayList<SyntaxError> getErrors() const { return errors; }

private:
    const ArrayList<Token> &tokens;
    size_t pos = 0;
    ArrayList<SyntaxError> errors;
    string currentContext;

    Token peek();
    Token get();
    bool match(TokenType type);
    bool match(const string &value);
    bool isAtEnd();
    void error(const string& message);
    void setContext(const string& context);
    
    void program();
    void block();
    void declarations();
    void statement();
    void assignmentOrCall();
    
    Token expectIdentifier();
    void expect(const string &value);
    void expectSemicolon();
    void expectEnd();
    void recoverTo(const string& delimiter);
};

#endif