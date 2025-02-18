#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>

#include "errors.h"
#include "lexer/tokenizer.h"
#include "parser.h"

using namespace Compiler;

using namespace Compiler::Parsing;

namespace Compiler {

std::string read_file_from_path(std::string path)
{
    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        if (!input) {
            std::cerr << "File does not exists" << std::endl;
            return "\0";
        }

        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    return contents;
}

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

} // namespace Compiler

int main(int argc, char* argv[])
{
    // If there is no arguments, then it's a line by line interactive interpreter
    if (argc < 2) {
        for (;;) {
            std::cout << "SQL>>" << std::endl;
            if (print(eval(read_input_from_user())) == "\0") {
                break;
            }
        }
    }
    // else this is a whole file that will be interpreted
    return 0;
}
