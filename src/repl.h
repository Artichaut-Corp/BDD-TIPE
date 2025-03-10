#ifndef REPL_H
#define REPL_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "errors.h"
#include "parser.h"

namespace Database::Utils {

class Repl {
    // Get input and lex
    static std::string Read()
    {
        std::string buf;
        getline(std::cin, buf);

        const std::string& ret = (buf == "\r\n" || buf == "\n") ? "\0" : buf;

        return ret;
    }

    static std::string Eval(const std::string& input)
    {
        if (input == "\0") {
            return "\0";
        }

        std::string output = "";

        Parsing::Parser* parser = new Parsing::Parser(input);

        std::variant<Parsing::Statement, Errors::Error> n = parser->Parse();

        if (std::holds_alternative<Errors::Error>(n)) {
            Errors::Error e = std::get<Errors::Error>(n);
            e.printAllInfo();

            return "\0";
        }

        auto stmt = std::get<Parsing::Statement>(n);

        // Traiter chaque Statement
        if (std::holds_alternative<Parsing::SelectStmt*>(stmt)) {
            auto select = std::get<Parsing::SelectStmt*>(stmt);

            delete select;

            output = "SELECT";
        } else if (std::holds_alternative<Parsing::UpdateStmt*>(stmt)) {
            auto update = std::get<Parsing::UpdateStmt*>(stmt);

            delete update;

            output = "UPDATE";
        } else if (std::holds_alternative<Parsing::InsertStmt*>(stmt)) {
            auto insert = std::get<Parsing::InsertStmt*>(stmt);

            delete insert;

            output = "INSERT";
        } else if (std::holds_alternative<Parsing::DeleteStmt*>(stmt)) {
            // delete est un mot réservé en C++
            auto del = std::get<Parsing::DeleteStmt*>(stmt);

            delete del;

            output = "DELETE";
        } else {
            throw std::runtime_error("Unrecognized Statement");
        }

        delete parser;

        return output;
    }

    static std::string Print(const std::string& input)
    {
        if (input == "\0") {
            return "\0";
        }

        std::cout << input << std::endl;

        return input;
    }

public:
    static void Run()
    {
        for (;;) {
            std::cout << "SQL>>" << std::endl;

            if (Print(Eval(Read())) == "\0") {
                break;
            }
        }
    }
};

}

#endif
