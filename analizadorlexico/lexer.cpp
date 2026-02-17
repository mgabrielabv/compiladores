#include "lexer.h"
#include <fstream>
#include <iostream>
#include <cctype>
using namespace std;
//diccionario de palabras reservadas
unordered_map<string, TokenType> Lexer::reservedWords;

void Lexer::loadReservedWords(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cout << "No se pudo abrir: " << filename << endl;
        exit(1);
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Ignorar lineas vacias o comentarios
        
        size_t espacio = line.find(' '); // busca la posicion del espacio
        if (espacio != string::npos) {  // Si hay un espacio, separamos la palabra y su tipo
            string word = line.substr(0, espacio);
            string type = line.substr(espacio + 1);
            
            if (type == "keyword") reservedWords[word] = TokenType::Keyword;
            else if (type == "operator") reservedWords[word] = TokenType::Operator;
            else if (type == "function") reservedWords[word] = TokenType::Function;
            else if (type == "delimiter") reservedWords[word] = TokenType::Delimiter;
            else reservedWords[word] = TokenType::Unknown;
        }
    }
}

void Lexer::initReservedWords() { // Inicializa el diccionario de palabras reservadas con valores predeterminados
    reservedWords.clear();
    
    // Palabras clave
    reservedWords["program"] = TokenType::Keyword;
    reservedWords["begin"] = TokenType::Keyword;
    reservedWords["end"] = TokenType::Keyword;
    reservedWords["var"] = TokenType::Keyword;
    reservedWords["integer"] = TokenType::Keyword;
    reservedWords["string"] = TokenType::Keyword;
    
    // Funciones
    reservedWords["writeln"] = TokenType::Function;
    
    // Delimitadores
    string delimiters = ";.,:(){}'";
    for (char c : delimiters) {
        reservedWords[string(1, c)] = TokenType::Delimiter;
    }
    
    // Operadores
    string operators[] = {":=", "+", "-", "*", "/", "=", "<>", "<", ">", "<=", ">=", "(*", "*)"};
    for (string op : operators) {
        reservedWords[op] = TokenType::Operator;
    }
}

//funciones basicas
Lexer::Lexer(const string &src) : input(src), pos(0), line(1), column(1) {} // constructor que recibe el codigo fuente como una cadena

char Lexer::peek() const {
    return (pos < input.size()) ? input[pos] : '\0';
}// si todavia hay caracteres por leer, devuelve el siguiente, sino devuelve el caracter nulo

char Lexer::get() {
    char c = peek();
    pos++;
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) get();
}

bool Lexer::isDelimiter(char c) const {
    string s(1, c);
    auto it = reservedWords.find(s);
    return (it != reservedWords.end() && it->second == TokenType::Delimiter);
}

//funcion principal del lexer que analiza el codigoy devuelve el siguiente token encontrado
Token Lexer::nextToken() {
    // salta espacios
    skipWhitespace();
    
    // guarda la posición actual para el token
    int startLine = line;
    int startColumn = column;
    
    // fin del archivo
    if (pos >= input.size()) {
        return {TokenType::EndOfFile, "", startLine, startColumn};
    }
    
    // 4. Ver el primer carácter
    char c = peek();
  //comentarios
    if (c == '{') {
        // Comentario { ... }
        while (peek() != '}' && peek() != '\0') get();
        if (peek() == '}') get();
        return nextToken(); // Ignorar y seguir
    }
    
    if (c == '(' && peekNext() == '*') {
        // Comentario (* ... *)
        get(); get(); // Consumir '(' y '*'
        while (!(peek() == '*' && peekNext() == ')') && peek() != '\0') {
            get();
        }
        if (peek() == '*') { get(); get(); } // Consumir '*' y ')'
        return nextToken();
    }
    //cadenas
    if (c == '\'') {
        // Cadena con comillas simples: ''
        get(); // Consumir '
        string value;
        while (peek() != '\'' && peek() != '\0') {
            value += get();
        }
        if (peek() == '\'') get(); // Consumir '
        return {TokenType::String, value, startLine, startColumn};
    }
    
    if (c == '"') {
        // Cadena con comillas dobles: ""
        string value;
        value += get(); // "
        while (peek() != '"' && peek() != '\0') {
            value += get();
        }
        if (peek() == '"') value += get(); // "
        return {TokenType::String, value, startLine, startColumn};
    }
    //caso 3 numeros
    if (isdigit(c)) {
        string value;
        while (isdigit(peek())) {
            value += get();
        }
        return {TokenType::Number, value, startLine, startColumn};
    }
    //caso 4 operadores de dos caracteres
    string twoChars = string(1, c) + peekNext();
    if (reservedWords.count(twoChars)) {
        get(); get(); // Consumir ambos
        return {reservedWords[twoChars], twoChars, startLine, startColumn};
    }
    
    // caso 5: operadores de un caracter
    string oneChar(1, c);
    if (reservedWords.count(oneChar)) {
        get(); // Consumir
        return {reservedWords[oneChar], oneChar, startLine, startColumn};
    }
    // caso 6: palabras reservadas o identificadores
    if (isalpha(c) || c == '_') {
        string value;
        while (isalnum(peek()) || peek() == '_') {
            value += get();
        }
        
        // es palabra reservada?
        if (reservedWords.count(value)) {
            return {reservedWords[value], value, startLine, startColumn};
        }
        
        // Si no, es identificador
        return {TokenType::Identifier, value, startLine, startColumn};
    }
    
    // caso 7: caracter desconocido
    string unknown(1, get());
    return {TokenType::Unknown, unknown, startLine, startColumn};
}
// Funcion peekNext()

char Lexer::peekNext() const {
    return (pos + 1 < input.size()) ? input[pos + 1] : '\0';
}