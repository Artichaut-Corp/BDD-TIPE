#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <variant>

#include "errors.h"

namespace Compiler::Utils {

class Repl {
    // Get input and lex
    std::string read_input_from_user()
    {
        std::string buf;
        getline(std::cin, buf);

        std::string ret = (buf == "\r\n" || buf == "\n") ? "\0" : buf;

        return ret;
    }

    std::string eval(std::string input)
    {
        if (input == "\0") {
            return "\0";
        }

        Parser* parser = new Parsing::Parser(input);

        auto n = parser->Parse();

        if (std::holds_alternative<Errors::Error>(n)) {
            Errors::Error e = std::get<Errors::Error>(n);
            e.printAllInfo();
        }
        return input;
    }

    std::string print(std::string input)
    {
        if (input == "\0") {
            return "\0";
        }

        return input;
    }
};

}

#endif
