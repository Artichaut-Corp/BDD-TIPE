#include <iostream>
#include <string>

#include "errors.h"

#ifndef REPL_H

#define REPL_H

namespace Database::Utils {

class Repl {
public:
    // Get input and lex
    static std::string Read()
    {
        std::string buf;
        getline(std::cin, buf);

        const std::string& ret = (buf == "\r\n" || buf == "\n") ? "\0" : buf;

        return ret;
    }

   
    // Pourrait prendre un type Result qui serait formatté et affiché ici
    static void Print(const std::string& result)
    {
        std::cout << result << std::endl;
    }
};

}

#endif
