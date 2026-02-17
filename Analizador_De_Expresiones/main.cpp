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

// Función para validar si la expresión tiene errores de sintaxis comunes
bool validarExpresion(const string &expr, string &errorMsg) {
    // Verificar si hay números seguidos de paréntesis sin operador (ej: 2(x+8))
    for (size_t i = 0; i < expr.length() - 1; i++) {
        if (isdigit(expr[i]) && expr[i+1] == '(') {
            errorMsg = "Error de sintaxis: falta operador entre numero y parentesis";
            return false;
        }
        if (isalpha(expr[i]) && expr[i+1] == '(') {
            // Esto podría ser una función, pero por ahora lo tratamos como error
            if (expr[i] != 'v' && expr[i] != 'x' && expr[i] != 'y') {
                // Solo para este ejemplo, asumimos que v, x, y son variables
            }
        }
    }
    return true;
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001); // Para caracteres especiales en Windows
    #endif

    cout << "=== ANALIZADOR DE EXPRESIONES ===\n";
    cout << "Comandos:\n";
    cout << "  variable = expresion  - Asignacion\n";
    cout << "  expresion             - Evaluacion\n";
    cout << "  salir                 - Terminar\n\n";

    while (true) {
        cout << "> ";
        string input;
        getline(cin, input);
        
        if (input.empty()) continue;
        if (input == "salir") break;

        // Corregir automáticamente si falta el operador
        string exprCorregida = input;
        size_t pos;
        while ((pos = exprCorregida.find(")(")) != string::npos) {
            exprCorregida.replace(pos, 2, ")*(");
        }
        
        // Corregir número seguido de paréntesis
        for (int i = exprCorregida.length() - 1; i > 0; i--) {
            if (isdigit(exprCorregida[i]) && exprCorregida[i+1] == '(') {
                exprCorregida.insert(i+1, "*");
            }
        }

        try {
            cout << "\nExpresión original: " << input << endl;
            if (input != exprCorregida) {
                cout << "Expresión corregida: " << exprCorregida << endl;
            }
            
            // Tokenizar la entrada
            ArrayList<Token> tokens = tokenizarExpresion(exprCorregida);
            
            // Parsear
            ExpressionParser parser(tokens);
            ExprNode *arbol = parser.parse();

            // Mostrar árbol en formato jerárquico como en la imagen
            cout << "\nÁrbol sintáctico:" << endl;
            imprimirArbolComoImagen(arbol, "", true);

            // Evaluar
            ExpressionEvaluator evaluator;
            
            if (arbol->valor == "ASSIGN") {
                evaluator.ejecutarAsignacion(arbol);
                cout << "\nAsignación realizada\n";
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