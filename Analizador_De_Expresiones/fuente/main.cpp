
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN 
    #endif
    #include <windows.h>
#endif

#include <iostream>
#include <string>

#include "../incluye/expression_parser.h"


using namespace std;

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001); 
    #endif

    cout << "Ingrese expresion: ";
    string input;
    getline(cin, input);
    if (input.empty()) return 0;

    try {
        ArrayList<Token> tokens = tokenizarExpresion(input);
        ExpressionParser parser(tokens);
        ExprNode *arbol = parser.parse();

        cout << "\nEstructura del Arbol:" << endl;
        imprimirArbol(arbol, "", true);

        ExpressionEvaluator evaluator;
        cout << "\nEjecucion:" << endl;
        if (arbol->valor == "ASSIGN") {
            evaluator.ejecutarAsignacion(arbol);
            evaluator.imprimirVariables();
        } else {
            cout << "Resultado: " << evaluator.evaluar(arbol) << endl;
        }
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}