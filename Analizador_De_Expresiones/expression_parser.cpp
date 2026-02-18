#include "expression_parser.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

using namespace std;

// Inicializa un nodo nuevo para construir el arbol de la expresion
ExprNode::ExprNode(const string &val, TokenType t, bool isParen)
    : valor(val), type(t), esParentesis(isParen) {}

// Prepara el analizador para leer la lista de tokens
ExpressionParser::ExpressionParser(const ArrayList<Token> &tokens) : tokens(tokens), pos(0) {}

bool ExpressionParser::isAtEnd() { 
    return pos >= tokens.size(); 
}

Token ExpressionParser::peek() { 
    if (isAtEnd()) {
        Token eof;
        eof.type = TokenType::EndOfFile;
        eof.value = "";
        eof.line = 0;
        eof.column = 0;
        return eof;
    }
    return tokens.get(pos); 
}

Token ExpressionParser::get() { 
    if (isAtEnd()) {
        Token eof;
        eof.type = TokenType::EndOfFile;
        eof.value = "";
        eof.line = 0;
        eof.column = 0;
        return eof;
    }
    return tokens.get(pos++); 
}

ExprNode *ExpressionParser::crearNodoTernario(const string &tipo, ExprNode *left, const string &op, ExprNode *right) {
    ExprNode *node = new ExprNode(tipo, TokenType::Identifier);
    node->izquierdo = left;
    node->operador = new ExprNode(op, TokenType::Operator);
    node->derecho = right;
    return node;
}

bool ExpressionParser::esVariableSimple(ExprNode *nodo) {
    if (!nodo) return false;
    return nodo->type == TokenType::Identifier && 
           nodo->izquierdo == nullptr && 
           nodo->derecho == nullptr &&
           nodo->operador == nullptr &&
           !nodo->esParentesis;
}

ExprNode *ExpressionParser::parse() { 
    return parseAssignment(); 
}

ExprNode *ExpressionParser::parseAssignment() {
    ExprNode *left = parseComparison();
    
    if (!isAtEnd() && peek().value == "=") {
        get(); // Saltamos el ' = ' porque ya sabemos que es una asignacion
        ExprNode *right = parseAssignment();
        
        if (!esVariableSimple(left)) {
            throw runtime_error("Error: Solo se puede asignar a variables");
        }
        
        return crearNodoTernario("ASSIGN", left, "=", right);
    }
    
    return left;
}

ExprNode *ExpressionParser::parseComparison() {
    ExprNode *left = parseAdditive();
    
    while (!isAtEnd() && (peek().value == "==" || peek().value == "!=" || 
                          peek().value == "<" || peek().value == ">" || 
                          peek().value == "<=" || peek().value == ">=")) {
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
    if (isAtEnd()) {
        throw runtime_error("Error: Expresion incompleta");
    }
    
    Token t = get();
    
    if (t.value == "(") {
        ExprNode *expr = parseAssignment();
        
        if (isAtEnd() || get().value != ")") {
            throw runtime_error("Error: Falta ')'");
        }
        
        ExprNode *res = new ExprNode("PAREN", TokenType::Identifier, true);
        res->operador = expr;
        return res;
    }
    
    if (t.type == TokenType::Number) {
        return new ExprNode(t.value, t.type);
    }
    
    if (t.type == TokenType::Identifier) {
        return new ExprNode(t.value, t.type);
    }
    
    throw runtime_error("Error: Token inesperado: " + t.value);
}


ExpressionEvaluator::ExpressionEvaluator() {}

double ExpressionEvaluator::pedirValorVariable(const string &nombre) {
    cout << "La variable '" << nombre << "' no esta definida." << endl;
    cout << "Ingrese un valor para " << nombre << ": ";
    
    string input;
    getline(cin, input);
    
    try {
        size_t pos;
        double valor = stod(input, &pos);
        if (pos < input.length()) {
            cout << "Procesando expresion para " << nombre << "..." << endl;
            
            ArrayList<Token> tokens = tokenizarExpresion(input);
            ExpressionParser parser(tokens);
            ExprNode *arbol = parser.parse();
            
            valor = evaluarNodo(arbol);
            
            delete arbol;
        }
        
        variables[nombre] = valor;
        cout << "  " << nombre << " = " << valor << " (guardado)" << endl;
        return valor;
        
    } catch (const exception &e) {
        cout << "Entrada no valida. Usando valor por defecto 0." << endl;
        variables[nombre] = 0;
        return 0;
    }
}

double ExpressionEvaluator::evaluarNodo(ExprNode *n) {
    if (!n) return 0;
    
    if (n->esParentesis) {
        return evaluarNodo(n->operador);
    }
    
    if (n->valor == "EXPR" || n->valor == "COMP") {
        double izquierdo = evaluarNodo(n->izquierdo);
        double derecho = evaluarNodo(n->derecho);
        string op = n->operador->valor;
        
        if (op == "+") return izquierdo + derecho;
        if (op == "-") return izquierdo - derecho;
        if (op == "*") return izquierdo * derecho;
        if (op == "/") {
            if (derecho == 0) throw runtime_error("Division por cero");
            return izquierdo / derecho;
        }
        if (op == "==") return izquierdo == derecho;
        if (op == "!=") return izquierdo != derecho;
        if (op == "<") return izquierdo < derecho;
        if (op == ">") return izquierdo > derecho;
        if (op == "<=") return izquierdo <= derecho;
        if (op == ">=") return izquierdo >= derecho;
    }
    
    if (n->valor == "ASSIGN") {
        throw runtime_error("Error: No se puede evaluar una asignacion directamente");
    }
    
    if (n->type == TokenType::Number) {
        return stod(n->valor);
    }
    
    if (n->type == TokenType::Identifier) {
        auto it = variables.find(n->valor);
        if (it != variables.end()) {
            return it->second;
        }
        return pedirValorVariable(n->valor);
    }
    
    return 0;
}

void ExpressionEvaluator::ejecutarAsignacion(ExprNode *n) {
    if (!n || n->valor != "ASSIGN") {
        throw runtime_error("Error: No es un nodo de asignacion");
    }
    
    string nombreVar = n->izquierdo->valor;
    cout << "Evaluando expresion para asignar a '" << nombreVar << "'..." << endl;
    double valor = evaluarNodo(n->derecho);
    variables[nombreVar] = valor;
    cout << "  " << nombreVar << " = " << valor << " (asignado)" << endl;
}

double ExpressionEvaluator::evaluar(ExprNode *n) {
    cout << "Evaluando expresion..." << endl;
    return evaluarNodo(n);
}

void ExpressionEvaluator::imprimirVariables() {
    if (variables.empty()) {
        cout << "No hay variables definidas" << endl;
        return;
    }
    
    cout << "\n--- Variables actuales ---" << endl;
    for (const auto &par : variables) {
        cout << "  " << setw(10) << left << par.first << " = " << par.second << endl;
    }
    cout << "--------------------------" << endl;
}

// --- HERRAMIENTAS ADICIONALES ---

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

// --- DIBUJAR EL ARBOL EN CONSOLA ---

void imprimirArbol(ExprNode *nodo, string prefijo, bool esUltimo) {
    if (!nodo) return;
    
    // Muestra la informacion del nodo que estamos visitando ahora
    cout << prefijo << (esUltimo ? "└── " : "├── ");
    
    // Decide que simbolo mostrar segun lo que sea el nodo (numero, suma, etc.)
    if (nodo->esParentesis) {
        cout << "( )";
    }
    else if (nodo->valor == "EXPR" || nodo->valor == "COMP") {
        if (nodo->operador) {
            cout << nodo->operador->valor;
        } else {
            cout << "?";
        }
    }
    else if (nodo->valor == "ASSIGN") {
        cout << "=";
    }
    else {
        cout << nodo->valor;
    }
    
    // Verificamos si es solo un numero para mostrarlo limpio
    if (nodo->type == TokenType::Number && nodo->valor != "EXPR" && nodo->valor != "COMP" && nodo->valor != "ASSIGN" && !nodo->esParentesis) {
        // Es un numero simple, asi que no hacemos nada especial aqui
    }
    else if (nodo->type == TokenType::Identifier && nodo->valor != "EXPR" && nodo->valor != "COMP" && nodo->valor != "ASSIGN" && !nodo->esParentesis) {
        cout << "";
    }
    
    cout << endl;
    
    // Preparamos las lineas (ramas) para dibujar los nodos hijos mas abajo
    string nuevoPrefijo = prefijo + (esUltimo ? "    " : "│   ");
    
    // Si es un parentesis, nos metemos adentro para dibujar lo que contiene
    if (nodo->esParentesis && nodo->operador) {
        imprimirArbol(nodo->operador, nuevoPrefijo, true);
        return;
    }
    
    // Si es una operacion, dibujamos los operandos de la izquierda y derecha
    if (nodo->izquierdo && nodo->derecho) {
        // Tiene valores a ambos lados
        imprimirArbol(nodo->izquierdo, nuevoPrefijo, false);
        imprimirArbol(nodo->derecho, nuevoPrefijo, true);
    }
    else if (nodo->izquierdo) {
        // Solo tiene algo a la izquierda
        imprimirArbol(nodo->izquierdo, nuevoPrefijo, true);
    }
    else if (nodo->derecho) {
        // Solo tiene algo a la derecha
        imprimirArbol(nodo->derecho, nuevoPrefijo, true);
    }
}

// Dibuja el arbol de una forma grafica bonita para entender la estructura
void imprimirArbolComoImagen(ExprNode *nodo, string prefijo, bool esUltimo) {
    if (!nodo) return;
    
    // Dibuja la rama y el nodo actual
    cout << prefijo << (esUltimo ? "└── " : "├── ");
    
    // Muestra que hay en este nodo (un signo, un numero, etc.)
    if (nodo->esParentesis) {
        cout << "( )";
    }
    else if (nodo->valor == "EXPR" || nodo->valor == "COMP") {
        if (nodo->operador) {
            cout << nodo->operador->valor;
        } else {
            cout << "?";
        }
    }
    else if (nodo->valor == "ASSIGN") {
        cout << "=";
    }
    else {
        cout << nodo->valor;
    }
    
    cout << endl;
    
    // Preparamos las lineas para los hijos
    string nuevoPrefijo = prefijo + (esUltimo ? "    " : "│   ");
    
    // Si es un parentesis, mostramos lo de adentro
    if (nodo->esParentesis && nodo->operador) {
        imprimirArbolComoImagen(nodo->operador, nuevoPrefijo, true);
        return;
    }
    
    // Si es una operacion, mostramos los dos lados
    if (nodo->izquierdo && nodo->derecho) {
        imprimirArbolComoImagen(nodo->izquierdo, nuevoPrefijo, false);
        imprimirArbolComoImagen(nodo->derecho, nuevoPrefijo, true);
    }
    else if (nodo->izquierdo) {
        imprimirArbolComoImagen(nodo->izquierdo, nuevoPrefijo, true);
    }
    else if (nodo->derecho) {
        imprimirArbolComoImagen(nodo->derecho, nuevoPrefijo, true);
    }
}