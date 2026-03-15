#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <vector>
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
    SemanticAnalyzer(const vector<Token> &tokens);
    void analyze();
    bool isSemanticallyCorrect() const;
    const vector<SemanticError> &getErrors() const;

private:
    const vector<Token> &tokens;
    unordered_map<string, SymbolInfo> symbolTable; // nombre -> info
    vector<SemanticError> errors;

    void checkVariableDeclaration();
    void checkAssignments();
    void checkConstants();
    void checkTypes();
    void checkControlStructures();

    bool isConditionBoolean(const vector<Token> &tokens, size_t start, size_t end) const;
    string getExpressionType(const Token &token) const;
    bool isValidComparison(const Token &op, const string &leftType, const string &rightType) const;
    bool isValidLogicalOperation(const Token &op, const string &leftType, const string &rightType) const;
    string getExpressionTypeForAssignment(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const;
    
    // Devuelve el índice final (no inclusivo) de la expresión, considerando paréntesis
    size_t getExpressionRange(size_t start, size_t maxEnd) const;
    
    // Evalúa el tipo de una condición en un rango y reporta errores si corresponde
    string getConditionTypeForRange(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const;
};

#endif // SEMANTIC_ANALYZER_H