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

void Parser::statement() {
    if (isAtEnd()) return;

    if (match(";")) return;

    if (match("program")) {
        Token progName = get();
        cout << "Cabecera del Programa: " << progName.value << endl;
        match(";");
        return;
    }


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
