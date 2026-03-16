#include "semantico.h"
#include <set>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer(const ArrayList<Token> &tokens)
    : tokens(tokens) {}

void SemanticAnalyzer::analyze()
{
    symbolTable.clear();
    errors = ArrayList<SemanticError>();

    checkVariableDeclaration();
    checkTypes();
    checkControlStructures();
}

bool SemanticAnalyzer::isSemanticallyCorrect() const
{
    return errors.size() == 0;
}

const ArrayList<SemanticError> &SemanticAnalyzer::getErrors() const
{
    return errors;
}

string SemanticAnalyzer::getExpressionType(const Token &token) const
{
    if (token.type == TokenType::Number)
    {
        return token.value.find('.') != string::npos ? "f64" : "i32";
    }

    if (token.type == TokenType::String)
    {
        return "String";
    }

    if (token.type == TokenType::Keyword)
    {
        if (token.value == "true" || token.value == "false")
            return "bool";
    }

    if (token.type == TokenType::Identifier)
    {
        auto it = symbolTable.find(token.value);
        if (it != symbolTable.end())
            return it->second.type;
    }

    return "";
}

void SemanticAnalyzer::checkVariableDeclaration()
{
    bool inVarSection = false;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].value == "var")
        {
            inVarSection = true;
            continue;
        }

        if (tokens[i].value == "begin")
        {
            break;
        }

        if (!inVarSection)
        {
            continue;
        }

        if (tokens[i].type != TokenType::Identifier)
        {
            continue;
        }

        ArrayList<string> names;
        size_t j = i;

        // Lee: a, b, c
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

        if (j >= tokens.size() || tokens[j].value != ":")
        {
            i = j;
            continue;
        }

        ++j;
        if (j >= tokens.size())
        {
            break;
        }

        string varType = tokens[j].value;
        if (varType == "integer")
            varType = "i32";
        else if (varType == "real")
            varType = "f64";
        else if (varType == "string")
            varType = "String";
        else if (varType == "boolean")
            varType = "bool";

        for (size_t n = 0; n < names.size(); ++n)
        {
            const string &name = names.get(n);
            if (symbolTable.count(name))
            {
                errors.add({tokens[i].line, tokens[i].column, "Variable redeclarada: " + name});
            }
            else
            {
                symbolTable[name] = SymbolInfo{varType, true};
            }
        }

        while (j < tokens.size() && tokens[j].value != ";" && tokens[j].value != "begin")
        {
            ++j;
        }

        i = j;
    }
}

size_t SemanticAnalyzer::getExpressionRange(size_t start, size_t maxEnd) const
{
    size_t i = start;
    int parenDepth = 0;

    while (i < maxEnd)
    {
        if (tokens[i].value == "(")
            ++parenDepth;
        else if (tokens[i].value == ")")
            --parenDepth;

        if (parenDepth <= 0 && (tokens[i].value == ";" || tokens[i].value == "," || tokens[i].value == "then" || tokens[i].value == "do"))
            break;

        ++i;
    }

    return i;
}

string SemanticAnalyzer::getExpressionTypeForAssignment(size_t start, size_t end, int &errorLine, int &errorCol, string &errorMsg) const
{
    if (start >= end)
        return "";

    // Caso decimal del lexer: 5 . 7
    if (end - start == 3 &&
        tokens[start].type == TokenType::Number &&
        tokens[start + 1].value == "." &&
        tokens[start + 2].type == TokenType::Number)
    {
        return "f64";
    }

    string currentType = getExpressionType(tokens[start]);
    if (tokens[start].type == TokenType::Identifier && currentType.empty())
    {
        errorLine = tokens[start].line;
        errorCol = tokens[start].column;
        errorMsg = "Variable no declarada: " + tokens[start].value;
        return "";
    }

    // Soporta expresiones binarias simples en cadena: a + b - c
    size_t i = start + 1;
    while (i + 1 < end)
    {
        const Token &op = tokens[i];
        const Token &rhsToken = tokens[i + 1];
        string rhsType = getExpressionType(rhsToken);

        if (rhsToken.type == TokenType::Identifier && rhsType.empty())
        {
            errorLine = rhsToken.line;
            errorCol = rhsToken.column;
            errorMsg = "Variable no declarada: " + rhsToken.value;
            return "";
        }

        if (op.value == "+" || op.value == "-" || op.value == "*" || op.value == "/")
        {
            // Regla 2: division entre 0
            if (op.value == "/" && rhsToken.type == TokenType::Number && (rhsToken.value == "0" || rhsToken.value == "0.0"))
            {
                errorLine = op.line;
                errorCol = op.column;
                errorMsg = "Error semantico: division entre 0";
                return "";
            }

            // Regla 4: no operar con String
            if (currentType == "String" || rhsType == "String")
            {
                errorLine = op.line;
                errorCol = op.column;
                errorMsg = "No se puede realizar operaciones aritmeticas con strings";
                return "";
            }

            bool leftNumeric = (currentType == "i32" || currentType == "f64");
            bool rightNumeric = (rhsType == "i32" || rhsType == "f64");
            if (!leftNumeric || !rightNumeric)
            {
                errorLine = op.line;
                errorCol = op.column;
                errorMsg = "Operacion aritmetica entre tipos incompatibles: " + currentType + " y " + rhsType;
                return "";
            }

            currentType = (currentType == "f64" || rhsType == "f64") ? "f64" : "i32";
        }
        else if (op.value == "==" || op.value == "!=" || op.value == "<" || op.value == ">" || op.value == "<=" || op.value == ">=")
        {
            if (currentType.empty() || rhsType.empty())
            {
                errorLine = op.line;
                errorCol = op.column;
                errorMsg = "Comparacion invalida entre " + currentType + " y " + rhsType;
                return "";
            }
            currentType = "bool";
        }
        else if (op.value == "&&" || op.value == "||")
        {
            if (currentType != "bool" || rhsType != "bool")
            {
                errorLine = op.line;
                errorCol = op.column;
                errorMsg = "Operacion logica invalida entre " + currentType + " y " + rhsType;
                return "";
            }
            currentType = "bool";
        }

        i += 2;
    }

    return currentType;
}

void SemanticAnalyzer::checkTypes()
{
    for (size_t i = 0; i + 2 < tokens.size(); ++i)
    {
        if (tokens[i].type != TokenType::Identifier)
            continue;

        if (tokens[i + 1].value != ":=")
            continue;

        const string varName = tokens[i].value;
        auto it = symbolTable.find(varName);
        if (it == symbolTable.end())
        {
            errors.add({tokens[i].line, tokens[i].column, "Variable no declarada: " + varName});
            continue;
        }

        size_t exprStart = i + 2;
        size_t exprEnd = getExpressionRange(exprStart, tokens.size());
        int errLine = 0, errCol = 0;
        string errMsg;
        string exprType = getExpressionTypeForAssignment(exprStart, exprEnd, errLine, errCol, errMsg);

        if (!errMsg.empty())
        {
            errors.add({errLine, errCol, errMsg});
            continue;
        }

        const string &varType = it->second.type;

        // Regla 1: integer no acepta String
        if (varType == "i32" && exprType == "String")
        {
            errors.add({tokens[exprStart].line, tokens[exprStart].column, "No se puede asignar un string a un entero"});
            continue;
        }

        // Regla 3: integer no acepta decimal
        if (varType == "i32" && exprType == "f64")
        {
            errors.add({tokens[exprStart].line, tokens[exprStart].column, "No se puede asignar un decimal a un entero"});
            continue;
        }

        if (!exprType.empty() && exprType != varType)
        {
            errors.add({tokens[exprStart].line, tokens[exprStart].column,
                        "Tipo incompatible en asignacion a '" + varName + "': se esperaba '" + varType + "', pero se obtuvo '" + exprType + "'"});
        }
    }
}

void SemanticAnalyzer::checkControlStructures()
{
    // Verifica que condiciones de if/while sean booleanas en forma simple
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].value != "if" && tokens[i].value != "while")
            continue;

        size_t condStart = i + 1;
        size_t condEnd = condStart;
        while (condEnd < tokens.size() && tokens[condEnd].value != "then" && tokens[condEnd].value != "do")
        {
            ++condEnd;
        }

        if (condStart >= condEnd)
            continue;

        int errLine = 0, errCol = 0;
        string errMsg;
        string condType = getExpressionTypeForAssignment(condStart, condEnd, errLine, errCol, errMsg);

        if (!errMsg.empty())
        {
            errors.add({errLine, errCol, errMsg});
            continue;
        }

        if (!condType.empty() && condType != "bool")
        {
            errors.add({tokens[condStart].line, tokens[condStart].column,
                        "La condicion del " + tokens[i].value + " debe ser una expresion booleana"});
        }
    }
}
