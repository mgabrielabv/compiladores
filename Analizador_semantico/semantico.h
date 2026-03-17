#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../LibreriaDeSoportes/arraylist.h"
#include <string>
#include <unordered_map>
#include "../analizadorlexico/lexer.h"

using namespace std;

struct SemanticError
{
    int line;
    int column;
    string message;
};

struct SymbolInfo
{
    string type;
    bool isMutable;
};

class SemanticAnalyzer
{
public:
    SemanticAnalyzer(const ArrayList<Token> &tokens);
    void analyze();
    bool isSemanticallyCorrect() const;
    const ArrayList<SemanticError> &getErrors() const;

private:
    const ArrayList<Token> &tokens;
    unordered_map<string, SymbolInfo> symbolTable; // nombre -> info
    ArrayList<SemanticError> errors;

    void checkVariableDeclaration();
    void checkTypes();
    void checkControlStructures();

    string getExpressionType(const Token &token) const;
    string getExpressionTypeForAssignment(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const;

    size_t getExpressionRange(size_t start, size_t maxEnd) const;
};

#endif 