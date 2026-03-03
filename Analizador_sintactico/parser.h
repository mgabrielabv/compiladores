#ifndef PARSER_H
#define PARSER_H

#include "../analizadorlexico/lexer.h" 
#include "../LibreriaDeSoportes/arraylist.h"
#include <string>

using namespace std;

class ExprNode; 
class ExpressionParser;

class Parser
{
public:
    Parser(const ArrayList<Token> &tokens);
    
    void parse(); 
    void loadOperators(const string &filename);
    bool hasSyntaxError = false;

private:
    const ArrayList<Token> &tokens;
    ArrayList<string> operators;
    size_t pos = 0; 

    Token peek();      
    Token get();       
    bool match(const string &expected); 
    bool isAtEnd();    

    void statement();
    void parseBlock();
    int evaluateCondition(ArrayList<Token> &condToken); 
    void parseBlockWithIndent(int indent);
    void imprimirArbolConIndentacion(ExprNode *node, int indent);
    ArrayList<Token> parseExpressionTokens(const string &delimiter);
};

#endif // PARSER_H