#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "lexer.h"
#include "arraylist.h"  
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

struct ExprNode {
    string valor;
    TokenType type;
    ExprNode *izquierdo = nullptr;
    ExprNode *operador = nullptr;
    ExprNode *derecho = nullptr;
    bool esParentesis = false;

    ExprNode(const string &val, TokenType t, bool isParen = false);
};

class ExpressionParser {
public:
    ExpressionParser(const ArrayList<Token> &tokens);
    ExprNode *parse();

private:
    const ArrayList<Token> &tokens;
    size_t pos = 0;
    bool isAtEnd();
    Token peek();
    Token get();
    ExprNode *crearNodoTernario(const string &tipo, ExprNode *left, const string &op, ExprNode *right);
    ExprNode *parsePrimary();
    ExprNode *parseAssignment();
    ExprNode *parseAdditive();
    ExprNode *parseComparison();
    ExprNode *parseMultiplicative();
    bool esVariableSimple(ExprNode *nodo);
};

class ExpressionEvaluator {
public:
    ExpressionEvaluator();
    double evaluar(ExprNode *nodo);
    void ejecutarAsignacion(ExprNode *nodo);
    void imprimirVariables();

private:
    double pedirValorVariable(const string &nombre);
    double evaluarNodo(ExprNode *nodo);
    map<string, double> variables;
};

ArrayList<Token> tokenizarExpresion(const string &expr);
void imprimirArbol(ExprNode *nodo, string prefijo = "", bool esUltimo = true);
void imprimirArbolComoImagen(ExprNode *nodo, string prefijo = "", bool esUltimo = true);  

#endif