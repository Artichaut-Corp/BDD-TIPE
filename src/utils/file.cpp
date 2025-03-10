#include "file.h"

#include <fcntl.h>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unistd.h>
#include <variant>

namespace Database::Utils {

std::variant<std::string, Errors::Error> readFileFromPathToString(const std::string& filePath)
{
    std::string fileContent;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        file.open(filePath);

        std::stringstream fileStream;

        fileStream << file.rdbuf();
        file.close();

        fileContent = fileStream.str();

    } catch (std::ifstream::failure err) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("File was not found with name {}", filePath), 0, 0, Errors::ERROR_FILE_NOT_FOUND);
    }

    return fileContent;
}

bool RecognizedDatabaseSignature(int fd)
{
    int buf[52];

    int e = read(fd, buf, 52 * sizeof(int));

    for (int i = 0; i < 52; i++) {

        if (buf[i] != signature[i]) {
            return false;
        }
    }

    return true;
}

void AddDatabaseSignature(int fd)
{
    int e = write(fd, signature, 52 * sizeof(int));
}

}
