#include "semantico.h"
#include <iostream>
#include <set>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer(const vector<Token> &tokens)
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

std::string SemanticAnalyzer::getExpressionType(const Token &token) const
{
    if (token.type == TokenType::Keyword)
    {
        if (token.value == "true" || token.value == "false")
            return "bool";
    }
    else if (token.type == TokenType::Number)
    {
        // Verificar si es un número con punto decimal (f64) o entero (i32)
        if (token.value.find('.') != std::string::npos)
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

bool SemanticAnalyzer::isValidComparison(const Token &op, const std::string &leftType, const std::string &rightType) const
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

bool SemanticAnalyzer::isValidLogicalOperation(const Token &op, const std::string &leftType, const std::string &rightType) const
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

bool SemanticAnalyzer::isConditionBoolean(const std::vector<Token> &tokens, size_t start, size_t end) const
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
                                std::string leftType = getExpressionType(tokens[j - 1]);
                                std::string rightType = getExpressionType(tokens[j + 1]);

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

const std::vector<SemanticError> &SemanticAnalyzer::getErrors() const
{
    return errors;
}

void SemanticAnalyzer::checkVariableDeclaration()
{
    // Busca declaraciones de variables y construye la tabla de símbolos
    symbolTable.clear();
    std::set<std::string> declared;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
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
                std::string varName = tokens[j].value;
                if (declared.count(varName))
                {
                    errors.push_back({tokens[j].line, tokens[j].column, "Variable redeclarada: " + varName});
                }
                else
                {
                    declared.insert(varName);
                    // Buscar tipo explícito
                    std::string varType = "";
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
                            std::string errMsg;
                            std::string exprType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);
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
                            std::string errMsg;
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
                                    if (tokens[exprStart].type == TokenType::Number && tokens[exprStart].value.find('.') != std::string::npos)
                                        varType = "f64";
                                    else if (tokens[exprStart].type == TokenType::String)
                                        varType = "String";
                                }
                            }
                        }
                        // Validar si el valor asignado es un identificador no declarado
                        if (tokens[exprStart].type == TokenType::Identifier)
                        {
                            std::string refName = tokens[exprStart].value;
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
    // Al final de checkVariableDeclaration(), agregar debug para mostrar la tabla de símbolos
    std::cout << "\n[DEBUG] Tabla de símbolos (nombre : tipo, mutable):\n";
    for (const auto &entry : symbolTable)
    {
        std::cout << "  " << entry.first << " : " << entry.second.type << ", mutable=" << (entry.second.isMutable ? "true" : "false") << std::endl;
    }
}

void SemanticAnalyzer::checkAssignments()
{
    // Verifica que las variables usadas en asignaciones estén declaradas y sean mutables
    std::set<std::string> assigned;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].value == "=")
        {
            std::string varName = tokens[i].value;
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
    std::set<std::string> consts;
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
            std::string varName = tokens[i].value;
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
            std::string varName = tokens[i].value;
            if (symbolTable.count(varName))
            {
                std::string varType = symbolTable[varName].type;
                size_t exprStart = i + 2;
                size_t exprEnd = getExpressionRange(exprStart, tokens.size());
                int errLine = 0, errCol = 0;
                std::string errMsg;
                std::string exprType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);
                if (!errMsg.empty())
                {
                    errors.push_back({errLine, errCol, errMsg});
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
                std::string leftType = getExpressionType(tokens[i - 1]);
                std::string rightType = getExpressionType(tokens[i + 1]);

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
                std::string leftType = getExpressionType(tokens[i - 1]);
                std::string rightType = getExpressionType(tokens[i + 1]);

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
                std::string leftType = getExpressionType(tokens[i - 1]);
                std::string rightType = getExpressionType(tokens[i + 1]);

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
std::string SemanticAnalyzer::getExpressionTypeForAssignment(size_t start, size_t end, int &errorLine, int &errorCol, std::string &errorMsg) const
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
        std::string leftType = getExpressionType(tokens[start]);
        std::string op = tokens[start + 1].value;
        std::string rightType = getExpressionType(tokens[start + 2]);
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
std::string SemanticAnalyzer::getConditionTypeForRange(size_t start, size_t end, int &errorLine, int &errorCol, std::string &errorMsg) const
{
    std::cout << "[DEBUG] getConditionTypeForRange: tokens[" << start << ", " << end << ")\n";
    for (size_t i = start; i < end; ++i)
    {
        std::cout << "  token: '" << tokens[i].value << "' tipo: " << (int)tokens[i].type << std::endl;
    }
    if (end <= start)
        return "";
    // Caso simple: un solo token
    if (end - start == 1)
    {
        std::string t = getExpressionType(tokens[start]);
        std::cout << "    [DEBUG] token simple: '" << tokens[start].value << "' tipo: " << t << std::endl;
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
            std::string op = tokens[i].value;
            std::cout << "    [DEBUG] operador: '" << op << "' en pos " << i << std::endl;
            if (op == "&&" || op == "||")
            {
                if (i > start && i + 1 < end)
                {
                    std::string leftType = getConditionTypeForRange(start, i, errorLine, errorCol, errorMsg);
                    std::string rightType = getConditionTypeForRange(i + 1, end, errorLine, errorCol, errorMsg);
                    std::cout << "    [DEBUG] op logico: leftType=" << leftType << ", rightType=" << rightType << std::endl;
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
                    std::string leftType = getConditionTypeForRange(start, i, errorLine, errorCol, errorMsg);
                    std::string rightType = getConditionTypeForRange(i + 1, end, errorLine, errorCol, errorMsg);
                    std::cout << "    [DEBUG] op comparacion: leftType=" << leftType << ", rightType=" << rightType << std::endl;
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
    std::string t = getExpressionType(tokens[start]);
    std::cout << "    [DEBUG] tipo final: '" << t << "'\n";
    if (t != "bool")
    {
        errorLine = tokens[start].line;
        errorCol = tokens[start].column;
        errorMsg = "La condición debe ser booleana, se obtuvo '" + t + "'";
    }
    return t;
}
