#include "parser.h"
#include "../analizadorlexico/lexer.h"
#include "../Analizador_De_Expresiones/expression_parser.h"
#include <iostream>
#include <stdexcept>

using namespace std;

Parser::Parser(const ArrayList<Token> &tokens) : tokens(tokens), pos(0) {
}

Token Parser::peek() { // mira el token actual sin avanzar
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos);
}

Token Parser::get() { // devuelve el token actual y avanza al siguiente
    return isAtEnd() ? Token{TokenType::EndOfFile, "", 0, 0} : tokens.get(pos++);
}

bool Parser::match(TokenType type) { // si es del tipo esperado, avanza
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

void Parser::error(const string& message) { // Cuando se encuentra un error, marca, guarda y dice el tipo de error
    if (hasSyntaxError) {
        return;
    }

    hasSyntaxError = true;
    Token current = peek();
    SyntaxError err;
    err.message = message;
    err.line = current.line;
    err.column = current.column;
    err.token = current.value;
    err.context = currentContext;
    errors.add(err);

    throw runtime_error("SYNTAX_ABORT");
}

void Parser::recoverTo(const string& delimiter) { //salta tokens hasta ver un ; o un end
    while (!isAtEnd() && peek().value != delimiter && 
           peek().value != ";" && peek().value != "end") {
        get();
    }
}

Token Parser::expectIdentifier() { // a que sea nombre de variable o funcion, sino da error y sigue buscando un token valido
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
 
void Parser::expect(const string &value) { // a que sea el token esperado, sino da error y sigue buscando un token valido
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
    try {
        program();
        if (!isAtEnd() && peek().type != TokenType::EndOfFile) {
            setContext("final del analisis");
            error("Tokens no procesados al final del archivo");
        }
    } catch (const runtime_error &e) {
        if (string(e.what()) != "SYNTAX_ABORT") {
            throw;
        }
    }

    if (hasSyntaxError) {
        cout << "hay un error sintactico\n";
    } else {
        cout << "Analisis sintactico completado: SIN ERRORES\n";
    }
}

void Parser::program() { // el programa debe empezar con program seguido de un identificador y un ; luego el bloque principal y terminar con .
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

void Parser::block() { // un bloque puede contener declaraciones de variables, procedimientos o funciones, seguido de un bloque principal entre begin end
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

void Parser::declarations() { // se analizan las variables declaradas, procedimientos o funciones
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
                    if (peek().type == TokenType::Unknown) { // si no es un token valido, da error y sigue buscando un token valido
                        Token t = get();
                        error("Token desconocido: '" + t.value + "' no es un identificador valido");
                        break;
                    }
                    
                    if (peek().type != TokenType::Identifier) { // si no es un identificador, da error y sigue buscando un token valido
                        Token t = get();
                        error("Se esperaba un identificador pero se encontro '" + t.value + "'");
                        break;
                    }
                    
                    Token id = get(); // si es un identificador valido, lo guarda
                    identifiers.add(id.value);
                    
                    if (peek().value == ",") { // si hay una coma, sigue buscando mas identificadores
                        get();
                    } else {
                        break;
                    }
                } while (!isAtEnd()); // si no es el final del bloque, sigue buscando mas identificadores
                
                if (identifiers.size() == 0) { 
                    while (!isAtEnd() && peek().value != ";" && peek().value != "begin") { // si no se encontraron identificadores salta hasta el siguiente ; o el inicio del bloque
                        get();
                    }
                    if (peek().value == ";") get();
                    continue;
                }
                
                expect(":");
                
                if (!isAtEnd()) { // si no es el final del bloque, espera un tipo de dato valido
                    Token tipo = peek();
                    
                    if (tipo.type == TokenType::Unknown) {
                        error("Tipo de dato no valido: '" + tipo.value + "'");
                        get();
                    } else if (tipo.type != TokenType::Type) {
                        error("Se esperaba un tipo de dato pero se encontro '" + tipo.value + "'");
                        get();
                    } else {
                        get();
                        cout << "  ";  // muestra los identificadores declarados con su tipo
                        for (size_t i = 0; i < identifiers.size(); i++) { 
                            if (i > 0) cout << ", ";
                            cout << identifiers.get(i);
                        }
                        cout << " : " << tipo.value << endl; 
                    }
                }
                
                expectSemicolon();
            }
         //si es una declaracion de procedimiento o funcion, espera tipo, nombre, parametros y bloque
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

void Parser::statement() { // maneja instrucciones tipo if, while, si no es ninguna de esas, asume que es una asignacion o llamada a procedimiento
    setContext("sentencia");
    
    if (isAtEnd()) return;
    
    if (peek().type == TokenType::Unknown) {
        Token t = get();
        error("Token desconocido: '" + t.value + "' no es una sentencia valida");
        recoverTo(";");
        return;
    }
    
    if (peek().value == ";") { //si encuentra un ; sin nada antes lo ignora y sigue buscando la siguiente sentencia
        get();
        return;
    }
    
    if (peek().value == "begin") {
        get();
        while (!isAtEnd() && peek().value != "end") { //si encuentra un token desconocido dentro del bloque error y sigue buscando una sentencia valida
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

    if (peek().value == "if") {
        get(); // consume 'if'

        ArrayList<Token> condition;
        while (!isAtEnd() && peek().value != "then") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido en condicion: '" + t.value + "'");
            } else {
                condition.add(get());
            }
        }

        if (peek().value == "then") {
            get();
        } else {
            error("Se esperaba 'then' en la sentencia if");
        }

        if (condition.size() > 0) {
            try {
                ExpressionParser exprParser(condition);
                ExprNode* exprTree = exprParser.parse();
            } catch (...) {
                error("Error en la expresion de condicion del if");
            }
        }

        statement(); // then

        if (peek().value == "else") {
            get();
            statement(); // else
        }
        return;
    }

    if (peek().value == "while") {
        get(); // 'while'

        ArrayList<Token> condition;
        while (!isAtEnd() && peek().value != "do") {
            if (peek().type == TokenType::Unknown) {
                Token t = get();
                error("Token desconocido en condicion: '" + t.value + "'");
            } else {
                condition.add(get());
            }
        }

        if (peek().value == "do") {
            get();
        } else {
            error("Se esperaba 'do' en la sentencia while");
        }

        if (condition.size() > 0) {
            try {
                ExpressionParser exprParser(condition);
                ExprNode* exprTree = exprParser.parse();
            } catch (...) {
                error("Error en la expresion de condicion del while");
            }
        }

        statement();
        return;
    }

    if (peek().value == "writeln" || peek().value == "readln") {
        string procName = peek().value;
        get(); // consume writeln/readln

        expect("(");
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
            error("Se esperaba ')' en llamada a " + procName);
        }

        expectSemicolon();
        return;
    }

    if (peek().type == TokenType::Identifier) { 
        assignmentOrCall();// si es un identificador,puede ser una asignacion o una llamada a procedimiento
        return;
    }
    
    Token t = get();
    error("Sentencia no valida: '" + t.value + "'");
    recoverTo(";");
}


void Parser::assignmentOrCall() { 
    Token ident = get();
    
    if (peek().value == ":=") { //si despues del identificador viene un := es una asignacion
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
        
        if (expression.size() > 0) {// si se encontro una expresion valida, se intenta parsear la expresion para detectar errores dentro de la expresion
            try {
                ExpressionParser exprParser(expression);
                ExprNode* exprTree = exprParser.parse();
            } catch (...) {
                error("Error en la expresion de asignacion");
            }
        }
        
        expectSemicolon();
        
    } else if (peek().value == "(") { //si viene un ( es una llamada a procedimiento, sino error
        get();

        // Validar que sea una llamada a un procedimiento permitido (por ahora solo writeln/readln)
        if (ident.value != "writeln" && ident.value != "readln") {
            error("Procedimiento no declarado: '" + ident.value + "'");
        }

        cout << "Llamada a: " << ident.value << endl; // muestra el nombre del procedimiento llamado
        
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
        error("Identificador '" + ident.value + "' no esperado en este contexto"); // si no es ni asignacion ni llamada a procedimiento, da error
        recoverTo(";");
    }
}