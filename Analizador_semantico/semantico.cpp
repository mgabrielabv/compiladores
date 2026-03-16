
#include "semantico.h"
#include <iostream>
#include <set>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer(const ArrayList<Token> &tokens)
    : tokens(tokens) {}

void SemanticAnalyzer::analyze()
{
    // Aquí se llamarán los métodos de chequeo semántico
    checkVariableDeclaration();
    checkAssignments();
    checkConstants();
    checkTypes();
    checkControlStructures();
}

string SemanticAnalyzer::getExpressionType(const Token &token) const
{
    if (token.type == TokenType::Keyword)
    {
        if (token.value == "true" || token.value == "false")
            return "bool";
    }
    else if (token.type == TokenType::Number)
    {
        // Verificar si es un número con punto decimal (f64) o entero (i32)
        if (token.value.find('.') != string::npos)
            return "f64";
        else
            return "i32";
    }
    else if (token.type == TokenType::String)
        return "String";
    else if (token.type == TokenType::Identifier)
    {
        auto it = symbolTable.find(token.value);
        if (it != symbolTable.end())
            return it->second.type;
    }
    return "";
}

bool SemanticAnalyzer::isValidComparison(const Token &op, const string &leftType, const string &rightType) const
{
    if (leftType.empty() || rightType.empty())
        return false;

    // Operadores de igualdad (==, !=) - permiten comparar cualquier tipo con su mismo tipo
    if (op.value == "==" || op.value == "!=")
    {
        return leftType == rightType;
    }

    // Operadores relacionales (<, >, <=, >=) - solo para números
    if (op.value == "<" || op.value == ">" || op.value == "<=" || op.value == ">=")
    {
        return (leftType == "i32" || leftType == "f64") &&
               (rightType == "i32" || rightType == "f64") &&
               leftType == rightType; // Deben ser del mismo tipo numérico
    }

    return false;
}

bool SemanticAnalyzer::isValidLogicalOperation(const Token &op, const string &leftType, const string &rightType) const
{
    if (leftType.empty() || rightType.empty())
        return false;

    // Operadores lógicos (&&, ||) - solo para booleanos
    if (op.value == "&&" || op.value == "||")
    {
        return leftType == "bool" && rightType == "bool";
    }

    return false;
}

bool SemanticAnalyzer::isConditionBoolean(const ArrayList<Token> &tokens, size_t start, size_t end) const
{
    if (start >= end)
        return false;

    // Caso simple: un solo valor booleano
    if (start == end - 1)
    {
        return getExpressionType(tokens[start]) == "bool";
    }

    // Buscar operadores booleanos
    for (size_t i = start; i < end; i++)
    {
        const Token &t = tokens[i];
        // Operadores de comparación
        if (t.value == "==" || t.value == "!=" || t.value == "<" ||
            t.value == ">" || t.value == "<=" || t.value == ">=")
        {
            return true;
        }
        // Operadores lógicos
        else if (t.value == "&&" || t.value == "||")
        {
            if (tokens[i - 1].value == "true" || tokens[i + 1].value == "false")
            {
                return true;
            }
        }
    }

    return false;
}

void SemanticAnalyzer::checkControlStructures()
{
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].value == "if" || tokens[i].value == "while")
        {
            if (i + 1 < tokens.size() && tokens[i + 1].value == "(")
            {
                size_t start = i + 2;
                size_t end = start;
                int depth = 1;

                // Encontrar el cierre del paréntesis
                while (end < tokens.size() && depth > 0)
                {
                    if (tokens[end].value == "(")
                        depth++;
                    if (tokens[end].value == ")")
                        depth--;
                    if (depth > 0)
                        end++;
                }

                if (depth == 0 && end > start)
                {
                    // Verificar si es una expresión booleana válida
                    if (!isConditionBoolean(tokens, start, end))
                    {
                        errors.push_back({tokens[start].line,
                                          tokens[start].column,
                                          "La condición del " + tokens[i].value + " debe ser una expresión booleana"});
                    }

                    // Verificar tipos en operadores de comparación
                    for (size_t j = start; j < end; j++)
                    {
                        if (tokens[j].value == "==" || tokens[j].value == "!=" || tokens[j].value == "<" ||
                            tokens[j].value == ">" || tokens[j].value == "<=" || tokens[j].value == ">=")
                        {
                            if (j > start && j + 1 < end)
                            {
                                string leftType = getExpressionType(tokens[j - 1]);
                                string rightType = getExpressionType(tokens[j + 1]);

                                if (!isValidComparison(tokens[j], leftType, rightType))
                                {
                                    errors.push_back({tokens[j].line,
                                                      tokens[j].column,
                                                      "Comparación inválida entre " + leftType + " y " + rightType +
                                                          " usando '" + tokens[j].value + "'"});
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool SemanticAnalyzer::isSemanticallyCorrect() const
{
    return errors.empty();
}

const ArrayList<SemanticError> &SemanticAnalyzer::getErrors() const
{
    return errors;
}

void SemanticAnalyzer::checkVariableDeclaration()
{
    // Busca declaraciones de variables y construye la tabla de símbolos
    symbolTable.clear();
    set<string> declared;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].value == "var")
        {
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j].value != "begin")
            {
                ArrayList<string> names;
                while (j < tokens.size() && tokens[j].type == TokenType::Identifier)
                {
                    names.add(tokens[j].value);
                    ++j;
                    if (j < tokens.size() && tokens[j].value == ",")
                    {
                        ++j;
                    }
                    else
                    {
                        break;
                    }
                }

                if (j < tokens.size() && tokens[j].value == ":")
                {
                    ++j;
                    if (j < tokens.size())
                    {
                        string varType = tokens[j].value;
                        if (varType == "integer")
                            varType = "i32";
                        else if (varType == "real")
                            varType = "f64";
                        else if (varType == "string")
                            varType = "String";

                        for (size_t n = 0; n < names.size(); ++n)
                        {
                            const string &varName = names.get(n);
                            if (declared.count(varName))
                            {
                                errors.push_back({tokens[j].line, tokens[j].column, "Variable redeclarada: " + varName});
                            }
                            else
                            {
                                declared.insert(varName);
                                symbolTable[varName] = SymbolInfo{varType, true};
                            }
                        }
                    }
                }

                while (j < tokens.size() && tokens[j].value != ";" && tokens[j].value != "begin")
                {
                    ++j;
                }
                if (j < tokens.size() && tokens[j].value == ";")
                {
                    ++j;
                }
            }
            continue;
        }

        if (tokens[i].value == "let" || tokens[i].value == "const")
        {
            bool isConst = tokens[i].value == "const";
            size_t j = i + 1;
            bool isMutable = false;
            if (j < tokens.size() && tokens[j].value == "mut")
            {
                isMutable = true;
                ++j; // let mut x
            }
            if (j < tokens.size() && tokens[j].type == TokenType::Identifier)
            {
                string varName = tokens[j].value;
                if (declared.count(varName))
                {
                    errors.push_back({tokens[j].line, tokens[j].column, "Variable redeclarada: " + varName});
                }
                else
                {
                    declared.insert(varName);
                    // Buscar tipo explícito
                    string varType = "";
                    size_t k = j + 1;
                    if (k < tokens.size() && tokens[k].value == ":")
                    {
                        if (k + 1 < tokens.size())
                        {
                            varType = tokens[k + 1].value;
                            k += 2; // Avanza sobre ':' y el tipo
                        }
                    }
                    if (k < tokens.size() && tokens[k].value == "=")
                    {
                        // Buscar el final de la expresión (el ';') considerando paréntesis
                        size_t exprStart = k + 1;
                        size_t exprEnd = getExpressionRange(exprStart, tokens.size());
                        // Validar tipo de la expresión completa si hay tipo explícito
                        if (!varType.empty() && exprStart < exprEnd)
                        {
                            int errLine = 0, errCol = 0;
                            string errMsg;
                            string exprType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);
                            if (!errMsg.empty())
                            {
                                errors.push_back({errLine, errCol, errMsg});
                            }
                            if (!exprType.empty() && exprType != varType)
                            {
                                errors.push_back({tokens[exprStart].line, tokens[exprStart].column, "Tipo incompatible en asignación a '" + varName + "': se esperaba '" + varType + "', pero se obtuvo '" + exprType + "'"});
                            }
                        }
                        // Solo inferir si NO hay tipo explícito
                        if (varType.empty() && exprStart < exprEnd)
                        {
                            int errLine = 0, errCol = 0;
                            string errMsg;
                            varType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);
                            if (!errMsg.empty())
                            {
                                errors.push_back({errLine, errCol, errMsg});
                            }
                            if (varType.empty())
                            {
                                varType = getExpressionType(tokens[exprStart]);
                                if (varType.empty())
                                {
                                    if (tokens[exprStart].type == TokenType::Number && tokens[exprStart].value.find('.') != string::npos)
                                        varType = "f64";
                                    else if (tokens[exprStart].type == TokenType::String)
                                        varType = "String";
                                }
                            }
                        }
                        // Validar si el valor asignado es un identificador no declarado
                        if (tokens[exprStart].type == TokenType::Identifier)
                        {
                            string refName = tokens[exprStart].value;
                            if (symbolTable.find(refName) == symbolTable.end())
                            {
                                errors.push_back({tokens[exprStart].line,
                                                  tokens[exprStart].column,
                                                  "Variable no declarada: " + refName});
                            }
                        }
                    }
                    symbolTable[varName] = SymbolInfo{varType, isConst ? false : isMutable};
                }
            }
        }
    }
}

void SemanticAnalyzer::checkAssignments()
{
    // Verifica que las variables usadas en asignaciones estén declaradas y sean mutables
    set<string> assigned;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].value == "=")
        {
            string varName = tokens[i].value;
            if (symbolTable.find(varName) == symbolTable.end())
            {
                errors.push_back({tokens[i].line, tokens[i].column, "Variable no declarada: " + varName});
            }
            else
            {
                // Si no es mutable y ya fue asignada antes, error
                if (!symbolTable[varName].isMutable && assigned.count(varName))
                {
                    errors.push_back({tokens[i].line, tokens[i].column, "No se puede reasignar variable inmutable: " + varName});
                }
                assigned.insert(varName);
            }
        }
    }
}

void SemanticAnalyzer::checkConstants()
{
    // Verifica que las constantes no sean reasignadas
    set<string> consts;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].value == "const")
        {
            size_t j = i + 1;
            if (j < tokens.size() && tokens[j].type == TokenType::Identifier)
            {
                consts.insert(tokens[j].value);
            }
        }
    }
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].value == "=")
        {
            string varName = tokens[i].value;
            if (consts.count(varName))
            {
                errors.push_back({tokens[i].line, tokens[i].column, "No se puede reasignar una constante: " + varName});
            }
        }
    }
}

void SemanticAnalyzer::checkTypes()
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].value == "=")
        {
                string varName = tokens[i].value;
            if (symbolTable.count(varName))
            {
                string varType = symbolTable[varName].type;
                size_t exprStart = i + 2;
                size_t exprEnd = getExpressionRange(exprStart, tokens.size());
                int errLine = 0, errCol = 0;
                string errMsg;
                string exprType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);
                if (!errMsg.empty())
                {
                    errors.push_back({errLine, errCol, errMsg});
                }
                // Regla 1: integer solo acepta enteros
                if (varType == "i32" && exprType == "String") {
                    errors.push_back({tokens[exprStart].line, tokens[exprStart].column, "No se puede asignar un string a un entero"});
                }
                // Regla 3: guardar decimal en entero
                if (varType == "i32" && exprType == "f64") {
                    errors.push_back({tokens[exprStart].line, tokens[exprStart].column, "No se puede asignar un decimal a un entero"});
                }
                // Regla 4: string en operación
                if (varType == "String" && (exprType == "i32" || exprType == "f64")) {
                    errors.push_back({tokens[exprStart].line, tokens[exprStart].column, "No se puede realizar operaciones aritméticas con strings"});
                }
                if (!exprType.empty() && exprType != varType)
                {
                    errors.push_back({tokens[exprStart].line, tokens[exprStart].column, "Tipo incompatible en asignación a '" + varName + "': se esperaba '" + varType + "', pero se obtuvo '" + exprType + "'"});
                }
            }
        }
    }

    // Validar tipos en operaciones aritméticas y comparaciones
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        // Operadores aritméticos
        if (tokens[i].type == TokenType::Operator &&
            (tokens[i].value == "+" || tokens[i].value == "-" || tokens[i].value == "*" || tokens[i].value == "/"))
        {
            if (i > 0 && i + 1 < tokens.size())
            {
                string leftType = getExpressionType(tokens[i - 1]);
                string rightType = getExpressionType(tokens[i + 1]);

                // Regla 2: división entre 0
                if (tokens[i].value == "/" && tokens[i + 1].type == TokenType::Number && tokens[i + 1].value == "0") {
                    errors.push_back({tokens[i].line, tokens[i].column, "Error semántico: división entre 0"});
                }

                // Verificar que ambos operandos estén declarados
                if (tokens[i - 1].type == TokenType::Identifier && leftType.empty())
                {
                    errors.push_back({tokens[i - 1].line, tokens[i - 1].column,
                                      "Variable no declarada: " + tokens[i - 1].value});
                }
                if (tokens[i + 1].type == TokenType::Identifier && rightType.empty())
                {
                    errors.push_back({tokens[i + 1].line, tokens[i + 1].column,
                                      "Variable no declarada: " + tokens[i + 1].value});
                }

                // Regla 4: string en operación
                if (leftType == "String" || rightType == "String") {
                    errors.push_back({tokens[i].line, tokens[i].column, "No se puede realizar operaciones aritméticas con strings"});
                }

                // Validar que ambos operandos sean del mismo tipo numérico
                if (leftType.empty() || rightType.empty())
                {
                    // Ya se reportó el error de variable no declarada arriba
                }
                else if (leftType != rightType)
                {
                    errors.push_back({tokens[i].line, tokens[i].column,
                                      "Operacion aritmetica entre tipos incompatibles: " + leftType + " y " + rightType});
                }
                else if (leftType != "i32" && leftType != "f64")
                {
                    errors.push_back({tokens[i].line, tokens[i].column,
                                      "Operacion aritmetica no permitida para tipo: " + leftType});
                }
            }
        }
        // Operadores de comparación
        else if (tokens[i].type == TokenType::Operator &&
                 (tokens[i].value == ">" || tokens[i].value == "<" || tokens[i].value == ">=" ||
                  tokens[i].value == "<=" || tokens[i].value == "==" || tokens[i].value == "!="))
        {
            if (i > 0 && i + 1 < tokens.size())
            {
                string leftType = getExpressionType(tokens[i - 1]);
                string rightType = getExpressionType(tokens[i + 1]);

                // Verificar que ambos operandos estén declarados
                if (tokens[i - 1].type == TokenType::Identifier && leftType.empty())
                {
                    errors.push_back({tokens[i - 1].line, tokens[i - 1].column,
                                      "Variable no declarada: " + tokens[i - 1].value});
                }
                if (tokens[i + 1].type == TokenType::Identifier && rightType.empty())
                {
                    errors.push_back({tokens[i + 1].line, tokens[i + 1].column,
                                      "Variable no declarada: " + tokens[i + 1].value});
                }

                if (leftType.empty() || rightType.empty())
                {
                    // Ya se reportó el error de variable no declarada arriba
                }
                else if (!isValidComparison(tokens[i], leftType, rightType))
                {
                    errors.push_back({tokens[i].line, tokens[i].column,
                                      "Comparacion invalida entre " + leftType + " y " + rightType +
                                          " usando '" + tokens[i].value + "'"});
                }
            }
        }
        // Operadores lógicos
        else if (tokens[i].type == TokenType::Operator &&
                 (tokens[i].value == "&&" || tokens[i].value == "||"))
        {
            if (i > 0 && i + 1 < tokens.size())
            {
                string leftType = getExpressionType(tokens[i - 1]);
                string rightType = getExpressionType(tokens[i + 1]);

                // Verificar que ambos operandos estén declarados
                if (tokens[i - 1].type == TokenType::Identifier && leftType.empty())
                {
                    errors.push_back({tokens[i - 1].line, tokens[i - 1].column,
                                      "Variable no declarada: " + tokens[i - 1].value});
                }
                if (tokens[i + 1].type == TokenType::Identifier && rightType.empty())
                {
                    errors.push_back({tokens[i + 1].line, tokens[i + 1].column,
                                      "Variable no declarada: " + tokens[i + 1].value});
                }

                if (leftType.empty() || rightType.empty())
                {
                    // Ya se reportó el error de variable no declarada arriba
                }
                else if (!isValidLogicalOperation(tokens[i], leftType, rightType))
                {
                    errors.push_back({tokens[i].line, tokens[i].column,
                                      "Operacion logica invalida entre " + leftType + " y " + rightType +
                                          " usando '" + tokens[i].value + "'"});
                }
            }
        }
    }
}

// Infiera el tipo de una expresión simple o binaria para validación de asignaciones
string SemanticAnalyzer::getExpressionTypeForAssignment(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const
{
    if (end <= start)
        return "";
    // Caso simple: un solo token
    if (end - start == 1)
    {
        return getExpressionType(tokens[start]);
    }
    // Caso binario: token op token
    if (end - start == 3)
    {
        string leftType = getExpressionType(tokens[start]);
        string op = tokens[start + 1].value;
        string rightType = getExpressionType(tokens[start + 2]);
        if (op == "+" || op == "-" || op == "*" || op == "/")
        {
            if (leftType.empty() || rightType.empty())
            {
                errorLine = tokens[start + 1].line;
                errorCol = tokens[start + 1].column;
                errorMsg = "Variable no declarada en operacion aritmetica";
                return "";
            }
            if (leftType != rightType)
            {
                errorLine = tokens[start + 1].line;
                errorCol = tokens[start + 1].column;
                errorMsg = "Operacion aritmetica entre tipos incompatibles: " + leftType + " y " + rightType;
                return "";
            }
            if (leftType == "i32" || leftType == "f64")
                return leftType;
            errorLine = tokens[start + 1].line;
            errorCol = tokens[start + 1].column;
            errorMsg = "Operacion aritmetica no permitida para tipo: " + leftType;
            return "";
        }
        if (op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=")
        {
            if (!isValidComparison(tokens[start + 1], leftType, rightType))
            {
                errorLine = tokens[start + 1].line;
                errorCol = tokens[start + 1].column;
                errorMsg = "Comparacion invalida entre " + leftType + " y " + rightType + " usando '" + op + "'";
                return "";
            }
            return "bool";
        }
        if (op == "&&" || op == "||")
        {
            if (!isValidLogicalOperation(tokens[start + 1], leftType, rightType))
            {
                errorLine = tokens[start + 1].line;
                errorCol = tokens[start + 1].column;
                errorMsg = "Operacion logica invalida entre " + leftType + " y " + rightType + " usando '" + op + "'";
                return "";
            }
            return "bool";
        }
    }
    // Para expresiones más complejas, no se infiere tipo
    return "";
}

// Devuelve el índice final (no inclusivo) de la expresión, considerando paréntesis
size_t SemanticAnalyzer::getExpressionRange(size_t start, size_t maxEnd) const
{
    size_t i = start;
    int parenDepth = 0;
    while (i < maxEnd)
    {
        if (tokens[i].value == "(")
            parenDepth++;
        if (tokens[i].value == ")")
            parenDepth--;
        if (parenDepth <= 0 && (tokens[i].value == ";" || tokens[i].value == ","))
            break;
        i++;
    }
    return i;
}

// Evalúa el tipo de una condición en un rango y reporta errores si corresponde
string SemanticAnalyzer::getConditionTypeForRange(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const
{
    if (end <= start)
        return "";
    // Caso simple: un solo token
    if (end - start == 1)
    {
        string t = getExpressionType(tokens[start]);
        if (t != "bool")
        {
            errorLine = tokens[start].line;
            errorCol = tokens[start].column;
            errorMsg = "La condición debe ser booleana, se obtuvo '" + t + "'";
        }
        return t;
    }
    // Buscar operadores lógicos y de comparación
    for (size_t i = start; i < end; ++i)
    {
        if (tokens[i].type == TokenType::Operator)
        {
            string op = tokens[i].value;
            if (op == "&&" || op == "||")
            {
                if (i > start && i + 1 < end)
                {
                    string leftType = getConditionTypeForRange(start, i, errorLine, errorCol, errorMsg);
                    string rightType = getConditionTypeForRange(i + 1, end, errorLine, errorCol, errorMsg);
                    if (!isValidLogicalOperation(tokens[i], leftType, rightType))
                    {
                        errorLine = tokens[i].line;
                        errorCol = tokens[i].column;
                        errorMsg = "Operacion logica invalida entre '" + leftType + "' y '" + rightType + "' usando '" + op + "'";
                        return "";
                    }
                    return "bool";
                }
            }
            else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
            {
                if (i > start && i + 1 < end)
                {
                    string leftType = getConditionTypeForRange(start, i, errorLine, errorCol, errorMsg);
                    string rightType = getConditionTypeForRange(i + 1, end, errorLine, errorCol, errorMsg);
                    if (!isValidComparison(tokens[i], leftType, rightType))
                    {
                        errorLine = tokens[i].line;
                        errorCol = tokens[i].column;
                        errorMsg = "Comparacion invalida entre '" + leftType + "' y '" + rightType + "' usando '" + op + "'";
                        return "";
                    }
                    return "bool";
                }
            }
        }
    }
    // Si no se encontró operador, intentar tipo simple
    string t = getExpressionType(tokens[start]);
    if (t != "bool")
    {
        errorLine = tokens[start].line;
        errorCol = tokens[start].column;
        errorMsg = "La condición debe ser booleana, se obtuvo '" + t + "'";
    }
    return t;
}
