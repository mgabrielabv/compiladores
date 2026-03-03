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

Parser::Parser(const ArrayList<Token> &tokens) : tokens(tokens), pos(0) {
    // Inicializar operadores estandar de Pascal para validaciones
    operators.add(":="); operators.add("="); operators.add("<>");
    operators.add("<"); operators.add("<="); operators.add(">");
    operators.add(">="); operators.add("+"); operators.add("-");
    operators.add("*"); operators.add("/"); operators.add("..");
}

Token Parser::peek() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos);
}

Token Parser::get() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos++);
}

bool Parser::match(const string &expected) {
    if (!isAtEnd() && peek().value == expected) {
        ++pos;
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return pos >= tokens.size();
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
        if (type == "Operator") operators.add(word);
    }
}

//Logica compleja de validacion de condiciones
int Parser::evaluateCondition(ArrayList<Token> &condToken) {
    for (int i = 0; i < (int)condToken.size() - 1; i++) {
        if (i % 2 != 0) {
            // Adaptado para operadores de Pascal como := o <>
            if ((condToken.get(i).value == ":" || condToken.get(i).value == "<" || condToken.get(i).value == ">" || condToken.get(i).value == "!") &&
                condToken.get(i + 1).value == "=") {
                condToken.get(i).value += condToken.get(i + 1).value;
                condToken.remove(i + 1);
            }
            else if ((condToken.get(i).value == "&" && condToken.get(i + 1).value == "&") ||
                     (condToken.get(i).value == "|" && condToken.get(i + 1).value == "|") ||
                     (condToken.get(i).value == "." && condToken.get(i + 1).value == ".")) {
                condToken.get(i).value += condToken.get(i + 1).value;
                condToken.remove(i + 1);
            }
        }
    }

    bool failed = false;
    bool failed2 = false;
    for (int i = 0; i < (int)condToken.size(); i++) {
        for (int j = 0; j < (int)operators.size(); j++) {
            if (i % 2 == 0) {
                failed = (condToken.get(i).value == operators.get(j));
            } else {
                failed2 = (condToken.get(i).value != operators.get(j));
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

    // Cabecera del PROGRAM (manejo opcional)
    if (match("program")) {
        Token progName = get();
        cout << "Cabecera del Programa: " << progName.value << endl;
        match(";");
        return;
    }

    //  Var (Pascal: var nombre : tipo ;)
    if (match("var")) {
        cout << "Declaracion de variables (VAR)\n";
        while (!isAtEnd() && peek().value != "begin" && peek().value != "procedure" && peek().value != "function") {
            if (isAtEnd()) break;
            Token ident = get(); 
            if (match(":")) {
                Token tipo = get();
                cout << "  Variable: " << ident.value << " : " << tipo.value << "\n";
            }
            match(";");
        }
        return;
    }

    if (match("procedure") || match("function")) {
        Token nombre = get();
        cout << "Declaracion de subprograma: " << nombre.value << "\n";
        if (match("(")) {
            while (!isAtEnd() && !match(")")) { get(); }
        }
        if (match(";")) {
             statement(); 
        }
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
        cout << "Estructura IF" << endl;
        
        ArrayList<Token> condTokens;
        while (!isAtEnd() && peek().value != "then") {
             condTokens.add(get());
        }
        match("then");
        
        cout << "  Condicion:" << endl;
        ExpressionParser exprParser(condTokens);
        try {
            ExprNode* exprTree = exprParser.parse();
            imprimirArbolConIndentacion(exprTree, 1);
        } catch(...) { cout << "Error en condicion if" << endl; }

        statement(); // Sentencia dentro del if
        
        if (match("else")) {
             cout << "Estructura ELSE" << endl;
             statement();
        }
        return;
    }
    
    // while/do
    if (match("while")) {
         cout << "Estructura WHILE" << endl;
         ArrayList<Token> condTokens;
         while (!isAtEnd() && peek().value != "do") {
             condTokens.add(get());
         }
         match("do");
         
         ExpressionParser exprParser(condTokens);
         //imprimirArbolConIndentacion(exprParser.parse(), 1);
         
         statement();
         return;
    }

    // Asignaciones o Llamadas a procedimiento
    if (peek().type == TokenType::Identifier) {
        Token ident = get();
        if (match(":=")) {
            cout << "Asignacion variable: " << ident.value << "\n";
            // Leer hasta ;
            ArrayList<Token> exprTokens = parseExpressionTokens(";");
            
            ExpressionParser exprParser(exprTokens);
            try {
                ExprNode* exprTree = exprParser.parse();
                imprimirArbolConIndentacion(exprTree, 4);
            } catch (const exception& e) {
                cout << "Error en asignacion: " << e.what() << endl;
            }
            return;
        } else if (match("(")) { 
            // Llamada a procedimiento como writeln(...)
            cout << "Llamada a procedimiento: " << ident.value << "\n";
             // consumir hasta )
             while(!isAtEnd() && peek().value != ")") get();
             match(")");
             match(";");
             return;
        }
    }
    
    // Consumo por defecto para evitar bucles infinitos si la estructura es desconocida
    if (!isAtEnd()) {
        Token t = get();
        cout << "Token ignorado o desconocido: " << t.value << endl;
    }
}

void Parser::parseBlock() {
    // Contenido del bloque hasta 'end'
    while (!isAtEnd() && peek().value != "end") {
        statement();
    }
    match("end");
    if (peek().value == "." || peek().value == ";") get(); 
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

ArrayList<Token> Parser::parseExpressionTokens(const string &delimiter) {
    ArrayList<Token> expr;
    while (!isAtEnd() && peek().value != delimiter) {
        expr.add(get());
    }
    if (peek().value == delimiter) {
        match(delimiter);
    }
    return expr;
}

void Parser::parseBlockWithIndent(int indent) {
    parseBlock();
}
