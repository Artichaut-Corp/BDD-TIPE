#include <iostream>
#include <string>
#include <vector>

#include "errors.h"

#ifndef REPL_H

#define REPL_H

namespace Database::Utils {

const std::vector<std::string> commands = { ".print_table_layout", ".insert_data" };

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

    static std::vector<std::string> completion_callback(const std::string& prefix, size_t)
    {
        std::vector<std::string> completions;
        for (auto const& cmd : commands) {
            if (cmd.find(prefix) == 0) {
                completions.push_back(cmd);
            }
        }
        return completions;
    }

    // Function for syntax highlighting
    static std::string highlighter_callback(const std::string& input)
    {
        std::string highlighted = input;

        // Highlight keywords in green
        for (const auto& cmd : commands) {
            size_t pos = 0;
            while ((pos = highlighted.find(cmd, pos)) != std::string::npos) {
                highlighted.replace(pos, cmd.length(), "\033[32m" + cmd + "\033[0m"); // ANSI green
                pos += cmd.length() + 9; // account for ANSI codes
            }
        }

        // Highlight the word "error" in red
        size_t pos = 0;
        while ((pos = highlighted.find("error", pos)) != std::string::npos) {
            highlighted.replace(pos, 5, "\033[31merror\033[0m"); // ANSI red
            pos += 13; // length of ANSI code
        }

        return highlighted;
    }
};

}

#endif
