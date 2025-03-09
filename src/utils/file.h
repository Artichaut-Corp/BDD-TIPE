#ifndef FILE_H
#define FILE_H

#include <string>
#include <variant>

#include "../errors.h"

namespace Compiler::Utils {

std::variant<std::string, Errors::Error> readFileFromPath(const std::string& filePath);

}

#endif
