#include "arraylist.h"
#include <iostream>
#include <string>

using namespace std;

class Persona {
public:
    string nombre;
    int edad;
    Persona() : nombre(""), edad(0) {}

    Persona(string n, int e) : nombre(n), edad(e) {}
};
ostream& operator<<(ostream& os, const Persona& p) {
    os << "Persona(Nombre: " << p.nombre << ", Edad: " << p.edad << ")";
    return os;
}

int main() {
    cout << "=== PRUEBAS COMPLETAS DE ARRAYLIST ===\n\n";
    
    // Probar con enteros
    cout << "1. PRUEBA CON ENTEROS\n";
    ArrayList<int> numeros;
    
    // add()
    cout << "   add(): ";
    numeros.add(10);
    numeros.add(20);
    numeros.add(30);
    cout << "Agregados 10, 20, 30\n";
    
    // size()
    cout << "   size(): " << numeros.size() << endl;
          
    // first() y last()
    cout << "   first(): " << numeros.first() << endl;
    cout << "   last(): " << numeros.last() << endl;
    
    // get()
    cout << "   get(1): " << numeros.get(1) << endl;
    cout << "   get(2): " << numeros.get(2) << endl;
    
    // Iteración con next()
    cout << "   Iteracion con next(): ";
    numeros.reset_iterator();
    int* elem;
    while ((elem = numeros.next()) != nullptr) {
        cout << *elem << " ";
    }
    cout << endl;
    
    // prev()
    cout << "   Iteracion con prev(): ";
    int* elem_prev;
    while ((elem_prev = numeros.prev()) != nullptr) {
        cout << *elem_prev << " ";
    }
    cout << endl;
    
    // remove()
    cout << "   remove(1): Eliminando elemento en posicion 1 (20)\n";
    numeros.remove(1);
    cout << "   size despues de remove: " << numeros.size() << endl;
    cout << "   Contenido actual: ";
    numeros.reset_iterator();
    while ((elem = numeros.next()) != nullptr) {
        cout << *elem << " ";
    }
    cout << endl;

    cout << "\n2. PRUEBA CON STRINGS\n";
    ArrayList<string> palabras;
    palabras.add("prueba");
    palabras.add("librerias");
    palabras.add("de soportes");
    
    cout << "   size(): " << palabras.size() << endl;
    cout << "   first(): " << palabras.first() << endl;
    cout << "   last(): " << palabras.last() << endl;
    
    cout << "   Iteracion: ";
    palabras.reset_iterator();
    string* palabra;
    while ((palabra = palabras.next()) != nullptr) {
        cout << *palabra << " ";
    }
    cout << endl;
    
    // Probar errores
    cout << "\n3. PRUEBA DE ERRORES\n";
    try {
        cout << "   Intentando get(10) en lista de 3 elementos: ";
        cout << palabras.get(10) << endl;
    } catch (const out_of_range& e) {
        cout << "Error esperado: " << e.what() << endl;
    }
    
    try {
        cout << "   Intentando remove(5) en lista de 3 elementos: ";
        palabras.remove(5);
    } catch (const out_of_range& e) {
        cout << "Error esperado: " << e.what() << endl;
    }
    
    try {
        ArrayList<int> vacia;
        cout << "   Intentando first() en lista vacia: ";
        cout << vacia.first() << endl;
    } catch (const out_of_range& e) {
        cout << "Error esperado: " << e.what() << endl;
    }
    cout << "\n4. PRUEBA CON OBJETOS\n";

    ArrayList<Persona> gente;

    cout << "   add(): Anadiendo objetos Persona\n";
    gente.add(Persona("Ana", 25));
    gente.add(Persona("Luis", 30));
    gente.add(Persona("Carlos", 22));

    cout << "   size(): " << gente.size() << endl;
    cout << "   first(): " << gente.first() << endl;
    cout << "   last(): " << gente.last() << endl;

    cout << "   Iteracion con next(): ";
    gente.reset_iterator();
    Persona* persona_ptr;
    while ((persona_ptr = gente.next()) != nullptr) {
        cout << *persona_ptr << " ";
    }
    cout << endl;
    
    return 0;
}