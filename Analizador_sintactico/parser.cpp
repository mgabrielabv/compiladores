#include "parser.h"
#include "../analizadorlexico/lexer.h"
#include "../Analizador_De_Expresiones/expression_parser.h"
#include <iostream>

using namespace std;

Parser::Parser(const ArrayList<Token> &tokens) : tokens(tokens), pos(0) {
}

Token Parser::peek() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos);
}

Token Parser::get() {
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos++);
}

bool Parser::match(TokenType type) {
    if (!isAtEnd() && peek().type == type) {
        ++pos;
        return true;
    }
    return false;
}

bool Parser::match(const string &value) {
    if (!isAtEnd() && peek().value == value) {
        ++pos;
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return pos >= tokens.size();
}

void Parser::setContext(const string& context) {
    currentContext = context;
}

void Parser::error(const string& message) {
    hasSyntaxError = true;
    Token current = peek();
    SyntaxError err;
    err.message = message;
    err.line = current.line;
    err.column = current.column;
    err.token = current.value;
    err.context = currentContext;
    errors.add(err);
    
    cerr << "Error sintactico en linea " << current.line 
         << ", columna " << curren3t.column << ": " << message << endl;
    if (!currentContext.empty()) {
        cerr << "  Contexto: " << currentContext << endl;
    }
}

void Parser::recoverTo(const string& delimiter) {
    while (!isAtEnd() && peek().value != delimiter && 
           peek().value != ";" && peek().value != "end") {
        get();
    }
}

Token Parser::expectIdentifier() {
    setContext("esperando identificador");
    
    if (isAtEnd()) {
        error("Se esperaba un identificador pero se encontro fin de archivo");
        return Token{TokenType::Unknown, "", 0, 0};
    }
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido: '" + t.value + "' no es un identificador valido");
        return t;
    }
    
    if (peek().type != TokenType::Identifier) {
        error("Se esperaba un identificador pero se encontro '" + peek().value + "'");
        return get();
    }
    
    return get();
}

void Parser::expect(const string &value) {
    setContext("esperando '" + value + "'");
    
    if (isAtEnd()) {
        error("Se esperaba '" + value + "' pero se encontro fin de archivo");
        return;
    }
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido: '" + t.value + "' se esperaba '" + value + "'");
        return;
    }
    
    if (peek().value != value) {
        error("Se esperaba '" + value + "' pero se encontro '" + peek().value + "'");
    } else {
        get();
    }
}

void Parser::expectSemicolon() {
    expect(";");
}

void Parser::expectEnd() {
    setContext("cerrando bloque");
    
    if (isAtEnd()) {
        error("Se esperaba 'end' para cerrar el bloque pero se encontro fin de archivo");
        return;
    }
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido: '" + t.value + "' se esperaba 'end'");
        
        while (!isAtEnd() && peek().value != "end") {
            if (peek().type == TokenType::Unknown) {
                get();
            } else {
                break;
            }
        }
    }
    
    if (peek().value != "end") {
        error("Se esperaba 'end' para cerrar el bloque pero se encontro '" + peek().value + "'");
        
        if (peek().value == "*" && pos + 1 < tokens.size() && tokens.get(pos + 1).value == ")") {
            error("Comentario mal formado: encontrado '*)' sin el correspondiente '(*'");
            get();
            get();
        }
        
        while (!isAtEnd() && peek().value != "end") {
            get();
        }
    }
    
    if (peek().value == "end") {
        get();
    }
}

void Parser::parse() {
    cout << "Iniciando analisis sintactico...\n";
    
    try {
        program();
    } catch (const exception &e) {
        error(string("Error fatal: ") + e.what());
    }
    
    if (!isAtEnd() && peek().type != TokenType::EndOfFile) {
        setContext("final del analisis");
        error("Tokens no procesados al final del archivo");
    }
    
    if (hasSyntaxError) {
        cout << "\n=========================================\n";
        cout << "Se encontraron " << errors.size() << " errores sintacticos:\n";
        cout << "=========================================\n";
        
        for (size_t i = 0; i < errors.size(); i++) {
            SyntaxError err = errors.get(i);
            cout << "Error " << (i+1) << ":\n";
            cout << "  Linea: " << err.line << ", Columna: " << err.column << "\n";
            cout << "  Token: '" << err.token << "'\n";
            cout << "  Mensaje: " << err.message << "\n";
            if (!err.context.empty()) {
                cout << "  Contexto: " << err.context << "\n";
            }
            cout << "\n";
        }
    } else {
        cout << "\nAnalisis sintactico completado: SIN ERRORES\n";
    }
}

void Parser::program() {
    setContext("cabecera del programa");
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido al inicio del programa: '" + t.value + "'");
    }
    
    if (peek().value == "program") {
        get();
        Token progName = expectIdentifier();
        cout << "Programa: " << progName.value << endl;
        expectSemicolon();
    } else {
        error("Se esperaba 'program' al inicio del programa");
    }
    
    block();
    
    setContext("final del programa");
    if (peek().value == ".") {
        get();
    } else {
        error("Se esperaba '.' al final del programa");
    }
}

void Parser::block() {
    declarations();
    
    setContext("cuerpo principal");
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido antes de 'begin': '" + t.value + "'");
    }
    
    if (peek().value == "begin") {
        get();
        cout << "Inicio de bloque BEGIN\n";
        
        while (!isAtEnd() && peek().value != "end") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido dentro del bloque: '" + t.value + "'");
                continue;
            }
            statement();
        }
        
        if (peek().value == "end") {
            get();
        } else {
            expectEnd();
        }
    } else {
        error("Se esperaba 'begin' para iniciar el bloque principal");
    }
}

void Parser::declarations() {
    setContext("declaraciones");
    
    while (!isAtEnd()) {
        if (peek().type == TokenType::Unknown) {
            Token t = get();
            error("Token desconocido en declaraciones: '" + t.value + "'");
            continue;
        }
        
        if (peek().value == "var") {
            get();
            cout << "Seccion VAR:\n";
            
            while (!isAtEnd() && peek().value != "begin" && 
                   peek().value != "procedure" && peek().value != "function") {
                
                ArrayList<string> identifiers;
                
                do {
                    if (peek().type == TokenType::Unknown) {
                        Token t = get();
                        error("Token desconocido: '" + t.value + "' no es un identificador valido");
                        break;
                    }
                    
                    if (peek().type != TokenType::Identifier) {
                        Token t = get();
                        error("Se esperaba un identificador pero se encontro '" + t.value + "'");
                        break;
                    }
                    
                    Token id = get();
                    identifiers.add(id.value);
                    
                    if (peek().value == ",") {
                        get();
                    } else {
                        break;
                    }
                } while (!isAtEnd());
                
                if (identifiers.size() == 0) {
                    while (!isAtEnd() && peek().value != ";" && peek().value != "begin") {
                        get();
                    }
                    if (peek().value == ";") get();
                    continue;
                }
                
                expect(":");
                
                if (!isAtEnd()) {
                    Token tipo = peek();
                    
                    if (tipo.type == TokenType::Unknown) {
                        error("Tipo de dato no valido: '" + tipo.value + "'");
                        get();
                    } else if (tipo.type != TokenType::Type) {
                        error("Se esperaba un tipo de dato pero se encontro '" + tipo.value + "'");
                        get();
                    } else {
                        get();
                        cout << "  ";
                        for (size_t i = 0; i < identifiers.size(); i++) {
                            if (i > 0) cout << ", ";
                            cout << identifiers.get(i);
                        }
                        cout << " : " << tipo.value << endl;
                    }
                }
                
                expectSemicolon();
            }
            
        } else if (peek().value == "procedure" || peek().value == "function") {
            string tipo = peek().value;
            get();
            Token nombre = expectIdentifier();
            cout << "Declaracion de " << tipo << ": " << nombre.value << endl;
            
            if (peek().value == "(") {
                get();
                while (!isAtEnd() && peek().value != ")") {
                    if (peek().type == TokenType::Unknown) {
                        get();
                    } else {
                        get();
                    }
                }
                if (peek().value == ")") get();
            }
            
            expectSemicolon();
            block();
            expectSemicolon();
            
        } else {
            break;
        }
    }
}

void Parser::statement() {
    setContext("sentencia");
    
    if (isAtEnd()) return;
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido: '" + t.value + "' no es una sentencia valida");
        recoverTo(";");
        return;
    }
    
    if (peek().value == ";") {
        get();
        return;
    }
    
    if (peek().value == "begin") {
        get();
        while (!isAtEnd() && peek().value != "end") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido dentro de bloque: '" + t.value + "'");
                continue;
            }
            statement();
        }
        expect("end");
        if (peek().value == ";") get();
        return;
    }
    
    if (peek().type == TokenType::Identifier) {
        assignmentOrCall();
        return;
    }
    
    Token t = get();
    error("Sentencia no valida: '" + t.value + "'");
    recoverTo(";");
}

void Parser::assignmentOrCall() {
    Token ident = get();
    
    if (peek().value == ":=") {
        get();
        cout << "Asignacion a: " << ident.value << endl;
        
        ArrayList<Token> expression;
        while (!isAtEnd() && peek().value != ";") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido en expresion: '" + t.value + "'");
            } else {
                expression.add(get());
            }
        }
        
        if (expression.size() > 0) {
            try {
                ExpressionParser exprParser(expression);
                ExprNode* exprTree = exprParser.parse();
            } catch (...) {
                error("Error en la expresion de asignacion");
            }
        }
        
        expectSemicolon();
        
    } else if (peek().value == "(") {
        get();
        cout << "Llamada a: " << ident.value << endl;
        
        while (!isAtEnd() && peek().value != ")") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido en parametros: '" + t.value + "'");
            } else {
                get();
            }
            if (peek().value == ",") get();
        }
        
        if (peek().value == ")") {
            get();
        } else {
            error("Se esperaba ')' en llamada a procedimiento");
        }
        
        expectSemicolon();
        
    } else {
        error("Identificador '" + ident.value + "' no esperado en este contexto");
        recoverTo(";");
    }
}