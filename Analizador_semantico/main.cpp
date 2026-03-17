#include "semantico.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../analizadorlexico/lexer.h"
#include "../Analizador_sintactico/parser.h"
#include "../Analizador_De_Expresiones/expression_parser.h"
#include "../LibreriaDeSoportes/arraylist.h"

using namespace std;

namespace
{
    ArrayList<Token> buildAssignmentExpression(const string &name, const ArrayList<Token> &expr)
    {
        ArrayList<Token> assignTokens;
        assignTokens.add(Token{TokenType::Identifier, name, 0, 0});
        assignTokens.add(Token{TokenType::Operator, "=", 0, 0});
        for (size_t i = 0; i < expr.size(); ++i)
        {
            assignTokens.add(expr.get(i));
        }
        return assignTokens;
    }

    void executeProgram(const ArrayList<Token> &tokens)
    {
        ExpressionEvaluator evaluator;

        size_t i = 0;
        while (i < tokens.size() && tokens[i].value != "begin")
            ++i;

        if (i == tokens.size())
            return;

        ++i;
        cout << "Resultados:\n";

        while (i < tokens.size() && tokens[i].value != "end")
        {
            if (tokens[i].type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].value == ":=")
            {
                string varName = tokens[i].value;
                i += 2;

                ArrayList<Token> expr;
                while (i < tokens.size() && tokens[i].value != ";")
                {
                    expr.add(tokens[i]);
                    ++i;
                }

                ArrayList<Token> assignExpr = buildAssignmentExpression(varName, expr);
                ExpressionParser exprParser(assignExpr);
                ExprNode *tree = exprParser.parse();
                evaluator.ejecutarAsignacion(tree);

                if (i < tokens.size() && tokens[i].value == ";")
                    ++i;
                continue;
            }

            if (tokens[i].value == "writeln")
            {
                ++i;
                if (i < tokens.size() && tokens[i].value == "(")
                    ++i;

                bool first = true;
                int parenDepth = 0;
                ArrayList<Token> currentExpr;

                while (i < tokens.size())
                {
                    if (tokens[i].value == "(")
                        ++parenDepth;
                    else if (tokens[i].value == ")")
                    {
                        if (parenDepth == 0)
                        {
                            if (currentExpr.size() > 0)
                            {
                                ExpressionParser exprParser(currentExpr);
                                ExprNode *tree = exprParser.parse();
                                double value = evaluator.evaluar(tree);
                                if (!first)
                                    cout << " ";
                                cout << value;
                                first = false;
                            }
                            ++i;
                            break;
                        }
                        --parenDepth;
                    }

                    if (tokens[i].value == "," && parenDepth == 0)
                    {
                        if (currentExpr.size() > 0)
                        {
                            ExpressionParser exprParser(currentExpr);
                            ExprNode *tree = exprParser.parse();
                            double value = evaluator.evaluar(tree);
                            if (!first)
                                cout << " ";
                            cout << value;
                            first = false;
                        }
                        currentExpr = ArrayList<Token>();
                        ++i;
                        continue;
                    }

                    currentExpr.add(tokens[i]);
                    ++i;
                }

                if (i < tokens.size() && tokens[i].value == ";")
                    ++i;

                cout << "\n";
                continue;
            }

            ++i;
        }
    }
}

int main()
{
    // Leer el código fuente Pascal desde el archivo code.pas
    ifstream file("../code.pas");
    if (!file)
    {
        cerr << "No se pudo abrir ../Analizador_sintactico/code.pas" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();

    // Inicializar palabras reservadas para Pascal
    Lexer::initReservedWords();

    // Tokenizar el código
    Lexer lexer(code);
    ArrayList<Token> tokens;
    Token token;

    do
    {
        token = lexer.nextToken();
        tokens.add(token);
    } while (token.type != TokenType::EndOfFile);

    // Analizar sintacticamente antes del semantico
    Parser parser(tokens);
    parser.parse();

    if (parser.hasSyntaxError)
    {
        return 1;
    }

    // Analizar semanticamente
    SemanticAnalyzer analyzer(tokens);
    analyzer.analyze();

    // Mostrar solo los errores semanticos
    if (!analyzer.isSemanticallyCorrect())
    {
        cout << "Errores semanticos encontrados:\n";
        for (size_t i = 0; i < analyzer.getErrors().size(); ++i)
        {
            const auto &error = analyzer.getErrors().get(i);
            cout << "Linea " << error.line << ", Columna " << error.column
                 << ": " << error.message << "\n";
        }
        return 1;
    }
    else
    {
        cout << "El codigo es semanticamente correcto.\n";
    }

    try
    {
        executeProgram(tokens);
    }
    catch (const exception &e)
    {
        cerr << "Error en ejecucion: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
