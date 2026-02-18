#include <iostream>
#include <string>
#include <sstream>
#include "expression_parser.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN 
    #endif
    #include <windows.h>
#endif

using namespace std;

// Funcion para validar si la expresion tiene errores de sintaxis comunes
bool validarExpresion(const string &expr, string &errorMsg) {
    for (size_t i = 0; i < expr.length() - 1; i++) {
        if (isdigit(expr[i]) && expr[i+1] == '(') {
            errorMsg = "Error de sintaxis: falta operador entre numero y parentesis";
            return false;
        }
    }
        
    return true;
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001); 
    #endif

    cout << "=== ANALIZADOR DE EXPRESIONES ===\n";
    cout << "Comandos:\n";
    cout << "  variable = expresion  - Asignacion\n";
    cout << "  expresion             - Evaluacion\n";
    cout << "  salir                 - Terminar\n\n";

    ExpressionEvaluator evaluator; 

    while (true) {
        cout << "> ";
        string input;
        getline(cin, input);
        
        if (input.empty()) continue;
        if (input == "salir") break;

        // Arregla el caso donde hay dos parentesis juntos: )( -> )*(
        string exprCorregida = input;
        size_t pos;
        while ((pos = exprCorregida.find(")(")) != string::npos) {
            exprCorregida.replace(pos, 2, ")*(");
        }
        
        // Agrega un multiplicador si hay un numero pegado a un parentesis
        for (int i = exprCorregida.length() - 1; i > 0; i--) {
            if (isdigit(exprCorregida[i]) && exprCorregida[i+1] == '(') {
                exprCorregida.insert(i+1, "*");
            }
        }

        try {
            cout << "\nExpresion original: " << input << endl;
            if (input != exprCorregida) {
                cout << "Expresion corregida: " << exprCorregida << endl;
            }
            
            ArrayList<Token> tokens = tokenizarExpresion(exprCorregida);
            
            // Convertimos el texto en una estructura de arbol para entenderlo
            ExpressionParser parser(tokens);
            ExprNode *arbol = parser.parse();

            // Dibujamos el arbol para que el usuario vea como se interpreto
            cout << "\nArbol sintactico:" << endl;
            imprimirArbolComoImagen(arbol, "", true);

            // Calculamos el resultado o hacemos la asignacion
            // ExpressionEvaluator evaluator; // Moved to outer scope
            
            if (arbol->valor == "ASSIGN") {
                evaluator.ejecutarAsignacion(arbol);
                cout << "\nAsignacion realizada\n";
                evaluator.imprimirVariables();
            } else {
                double resultado = evaluator.evaluar(arbol);
                cout << "\nResultado: " << resultado << endl;
                evaluator.imprimirVariables();
            }
            
            cout << endl;

        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl << endl;
        }
    }
    
    return 0;
}