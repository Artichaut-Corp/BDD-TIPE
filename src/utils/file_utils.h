#include <string>
#include <unordered_set>
#include <variant>

#include "../errors.h"

#ifndef FILE_UTILS_H

#define FILE_UTILS_H

namespace Database::Utils {

std::variant<std::string, Errors::Error> readFileFromPath(const std::string& filePath);


}

#endif //!FILE_UTILS_H
