#ifndef LEXER_PROYECTO_H
#define LEXER_PROYECTO_H
#include <string>
#include <unordered_map>

using namespace std;

enum class TokenType // enum define los tipos de tokens que el lexer puede reconocer
{
    Keyword,
    Operator,
    Function,
    Delimiter,
    Identifier,
    Number,
    String,
    Comment,
    Unknown,
    Type,
    EndOfFile
};

struct Token
{
    TokenType type; 
    string value; //valor del texto del token
    int line;
    int column;
};

class Lexer // clase que represanta el analizador lexico, encargado de convertir el codigo fuente en tokens
{
public:
    Lexer(const string &src); // constructor que recibe el codigo fuente como una cadena
    Token nextToken(); // analiza y devuelve el siguiente token
    static void loadReservedWords(const string &filename); 
    static void initReservedWords();

private:
    string input; //almacena el codigo fuente completo como un string
    size_t pos = 0; //empieza en 0
    int line = 1, column = 1;

    char peekNext() const; //sirve para mirar el siguiente caracter sin cambiar de posicion
    char peek() const; 
    char get(); // retorna y avanza
    void skipWhitespace();
    bool isDelimiter(char c) const; //verifica si un caracter es un delimitador
    static TokenType stringToTokenType(const string &str); // convierte un string en un tipo de token

    static unordered_map<string, TokenType> reservedWords;// guarda las palabras reservadas y sus tipos de token asociados
};
#endif