#include "../incluye/expression_parser.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;

ExprNode::ExprNode(const string &val, TokenType t, bool isParen)
    : valor(val), type(t), esParentesis(isParen) {}

ExpressionParser::ExpressionParser(const ArrayList<Token> &tokens) : tokens(tokens) {}

bool ExpressionParser::isAtEnd() { return pos >= tokens.size(); }
Token ExpressionParser::peek() { return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos); }
Token ExpressionParser::get() { return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos++); }

ExprNode *ExpressionParser::crearNodoTernario(const string &tipo, ExprNode *left, const string &op, ExprNode *right) {
    ExprNode *node = new ExprNode(tipo, TokenType::Identifier);
    node->izquierdo = left;
    node->operador = new ExprNode(op, TokenType::Operator);
    node->derecho = right;
    return node;
}

bool ExpressionParser::esVariableSimple(ExprNode *nodo) {
    return nodo != nullptr && nodo->operador == nullptr && nodo->izquierdo == nullptr && 
           nodo->derecho == nullptr && !nodo->esParentesis &&
           !all_of(nodo->valor.begin(), nodo->valor.end(), ::isdigit);
}

ExprNode *ExpressionParser::parse() { return parseAssignment(); }

ExprNode *ExpressionParser::parseAssignment() {
    ExprNode *left = parseComparison();
    if (peek().value == "=") {
        get();
        ExprNode *right = parseAssignment();
        if (!esVariableSimple(left)) throw runtime_error("Asignacion invalida");
        return crearNodoTernario("ASSIGN", left, "=", right);
    }
    return left;
}

ExprNode *ExpressionParser::parseComparison() {
    ExprNode *left = parseAdditive();
    while (!isAtEnd() && (peek().value == "==" || peek().value == "!=" || peek().value == "<" || peek().value == ">")) {
        string op = get().value;
        ExprNode *right = parseAdditive();
        left = crearNodoTernario("COMP", left, op, right);
    }
    return left;
}

ExprNode *ExpressionParser::parseAdditive() {
    ExprNode *left = parseMultiplicative();
    while (!isAtEnd() && (peek().value == "+" || peek().value == "-")) {
        string op = get().value;
        ExprNode *right = parseMultiplicative();
        left = crearNodoTernario("EXPR", left, op, right);
    }
    return left;
}

ExprNode *ExpressionParser::parseMultiplicative() {
    ExprNode *left = parsePrimary();
    while (!isAtEnd() && (peek().value == "*" || peek().value == "/")) {
        string op = get().value;
        ExprNode *right = parsePrimary();
        left = crearNodoTernario("EXPR", left, op, right);
    }
    return left;
}

ExprNode *ExpressionParser::parsePrimary() {
    Token t = get();
    if (t.value == "(") {
        ExprNode *expr = parseAssignment();
        if (get().value != ")") throw runtime_error("Falta ')'");
        ExprNode *res = new ExprNode("PAREN", TokenType::Identifier, true);
        res->operador = expr;
        return res;
    }
    return new ExprNode(t.value, t.type);
}

// --- EVALUADOR ---
ExpressionEvaluator::ExpressionEvaluator() {}
double ExpressionEvaluator::evaluarNodo(ExprNode *n) {
    if (!n) return 0;
    if (n->esParentesis) return evaluarNodo(n->operador);
    if (n->valor == "EXPR" || n->valor == "COMP") {
        double l = evaluarNodo(n->izquierdo), r = evaluarNodo(n->derecho);
        string op = n->operador->valor;
        if (op == "+") return l + r;
        if (op == "-") return l - r;
        if (op == "*") return l * r;
        if (op == "/") return r == 0 ? 0 : l / r;
        if (op == "==") return l == r;
    }
    if (isdigit(n->valor[0])) return stod(n->valor);
    return variables[n->valor];
}
void ExpressionEvaluator::ejecutarAsignacion(ExprNode *n) {
    variables[n->izquierdo->valor] = evaluarNodo(n->derecho);
}
double ExpressionEvaluator::evaluar(ExprNode *n) { return evaluarNodo(n); }
void ExpressionEvaluator::imprimirVariables() {
    for (auto const& [key, val] : variables) cout << key << " = " << val << endl;
}

// --- DIBUJO DEL ARBOL ---
void imprimirArbol(ExprNode *nodo, string prefijo, bool esUltimo) {
    if (!nodo) return;
    cout << prefijo << (esUltimo ? "└── " : "├── ");
    string label = (nodo->valor == "EXPR" || nodo->valor == "ASSIGN" || nodo->valor == "COMP") ? "[" + nodo->valor + "]" : nodo->valor;
    cout << label << (nodo->esParentesis ? " (paren)" : "") << endl;

    string nuevoPrefijo = prefijo + (esUltimo ? "    " : "│   ");
    int hijos = 0;
    if (nodo->izquierdo) hijos++;
    if (nodo->operador && nodo->valor != nodo->operador->valor) hijos++;
    if (nodo->derecho) hijos++;

    int cont = 0;
    if (nodo->izquierdo) imprimirArbol(nodo->izquierdo, nuevoPrefijo, (++cont == hijos));
    if (nodo->operador && nodo->valor != nodo->operador->valor) imprimirArbol(nodo->operador, nuevoPrefijo, (++cont == hijos));
    if (nodo->derecho) imprimirArbol(nodo->derecho, nuevoPrefijo, (++cont == hijos));
}

ArrayList<Token> tokenizarExpresion(const string &expr) {
    Lexer lexer(expr);
    ArrayList<Token> tokens;
    while (true) {
        Token t = lexer.nextToken();
        tokens.add(t);
        if (t.type == TokenType::EndOfFile) break;
    }
    return tokens;
}