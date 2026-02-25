#include "parser.h"
#include "../analizadorlexico/lexer.h"
#include "../Analizador_De_Expresiones/expression_parser.h" 
#include <iostream>
#include <fstream>
#include "parser.h"
#include <filesystem>
#include <sstream>
#include <functional>

using namespace std;

static ArrayList<Token> vectorToArrayList(const vector<Token> &vec) {
    ArrayList<Token> list;
    for (const auto &t : vec) {
        list.add(t);
    }
    return list;
}

Parser::Parser(const vector<Token> &tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens[pos];
}

Token Parser::get() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens[pos++];
}

bool Parser::match(const string &expected) {
    if (!isAtEnd() && peek().value == expected) {
        ++pos;
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return pos >= tokens.size() || (pos > 0 && tokens[pos-1].value == ".");
}

void Parser::parse() {
    while (!isAtEnd()) {
        statement();
    }
}

void Parser::loadOperators(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "No se pudo abrir el archivo de tokens: " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream iss(line);
        string word, type;
        iss >> word >> type;
        if (type == "Operator") operators.push_back(word);
    }
}

//Logica compleja de validacion de condiciones
int Parser::evaluateCondition(vector<Token> &condToken) {
    for (int i = 0; i < (int)condToken.size() - 1; i++) {
        if (i % 2 != 0) {
            // Adaptado para operadores de Pascal como := o <>
            if ((condToken[i].value == ":" || condToken[i].value == "<" || condToken[i].value == ">" || condToken[i].value == "!") &&
                condToken[i + 1].value == "=") {
                condToken[i].value += condToken[i + 1].value;
                condToken.erase(condToken.begin() + (i + 1));
            }
            else if ((condToken[i].value == "&" && condToken[i + 1].value == "&") ||
                     (condToken[i].value == "|" && condToken[i + 1].value == "|") ||
                     (condToken[i].value == "." && condToken[i + 1].value == ".")) {
                condToken[i].value += condToken[i + 1].value;
                condToken.erase(condToken.begin() + (i + 1));
            }
        }
    }

    bool failed = false;
    bool failed2 = false;
    for (int i = 0; i < (int)condToken.size(); i++) {
        for (int j = 0; j < (int)operators.size(); j++) {
            if (i % 2 == 0) {
                failed = (condToken[i].value == operators[j]);
            } else {
                failed2 = (condToken[i].value != operators[j]);
                if (!failed2) break;
            }
            if (failed) return -1;
        }
        if (failed2) return -1;
    }
    return 1;
}

void Parser::statement() {
    if (isAtEnd()) return;

    if (match(";")) return;

    //  Var
    if (match("var")) {
        cout << "Declaracion de variables (VAR)\n";
        while (!isAtEnd() && peek().value != "begin" && peek().value != "procedure") {
            Token ident = get();
            if (match(":")) {
                Token tipo = get();
                cout << "  Variable: " << ident.value << " : " << tipo.value << "\n";
            }
            match(";");
        }
        return;
    }

    // PROCEDURES / FUNCTIONS ---
    if (match("procedure") || match("function")) {
        Token nombre = get();
        cout << "Declaracion de subprograma: " << nombre.value << "\n";
        if (match("(")) {
            while (!isAtEnd() && !match(")")) { get(); }
        }
        if (match(";")) statement(); 
        return;
    }

    // begin/end
    if (match("begin")) {
        cout << "Inicio de bloque (BEGIN)\n";
        parseBlock();
        return;
    }

    // if/then/else
    if (match("if")) {
        cout << "if\n";
        vector<Token> condTokens;
        // En Pascal se busca el 'then'
        while (!isAtEnd() && peek().value != "then") {
            condTokens.push_back(get());
        }
        match("then");

        if (evaluateCondition(condTokens) == -1) {
            cout << "  El formato de la condicion no es valida" << endl;
        } else {
            try {
                // Convierte los tokens en ArrayList antes de pasarlos al analizador de expresiones
                ArrayList<Token> condList = vectorToArrayList(condTokens);
                ExpressionParser condParser(condList);
                ExprNode *cond = condParser.parse();
                if (cond) imprimirArbolConIndentacion(cond, 4);
            } catch (const runtime_error &e) {
                cerr << "Error de sintaxis: " << e.what() << endl;
                hasSyntaxError = true;
            }
        }
        statement();
        if (match("else")) statement();
        return;
    }

    // while/do
    if (match("while")) {
        cout << "while\n";
        vector<Token> condTokens = parseExpressionTokens("do");
        if (evaluateCondition(condTokens) == -1) {
             cout << "  Condicion invalida" << endl;
        } else {
            ArrayList<Token> condList = vectorToArrayList(condTokens);
            ExpressionParser condParser(condList);
            imprimirArbolConIndentacion(condParser.parse(), 4);
        }
        statement();
        return;
    }

    // asignaciones 
    if (peek().type == TokenType::Identifier) {
        Token ident = get();
        if (match(":=")) {
            cout << "asignacion variable: " << ident.value << "\n";
            vector<Token> exprTokens = parseExpressionTokens(";");
            ArrayList<Token> exprList = vectorToArrayList(exprTokens);
            ExpressionParser exprParser(exprList);
            imprimirArbolConIndentacion(exprParser.parse(), 4);
            return;
        }
        // Llamada a procedimiento
        if (match("(")) {
            parseExpressionTokens(")");
            match(";");
            return;
        }
    }

    // si nada coincide, se avanza
    get();
}

void Parser::parseBlock() {
    while (!isAtEnd() && peek().value != "end") {
        statement();
    }
    match("end");
    if (peek().value == ";" || peek().value == ".") get();
}

void Parser::imprimirArbolConIndentacion(ExprNode *node, int indent) {
    if (!node) return;
    string indentStr(indent, ' ');
    if (node->operador) {
        imprimirArbolConIndentacion(node->izquierdo, indent + 2);
        cout << indentStr << node->operador->valor << "\n";
        imprimirArbolConIndentacion(node->derecho, indent + 2);
    } else {
        cout << indentStr << node->valor << "\n";
    }
}

vector<Token> Parser::parseExpressionTokens(const string &delimiter) {
    vector<Token> expr;
    while (!isAtEnd() && peek().value != delimiter) {
        expr.push_back(get());
    }
    match(delimiter);
    return expr;
}

void Parser::parseBlockWithIndent(int indent) {
    string indentStr(indent, ' ');
    while (!isAtEnd() && peek().value != "end") {
        cout << indentStr;
        statement();
    }
    match("end");
}