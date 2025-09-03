#include "file.h"
#include "../storage.h"
#include "../utils.h"
#include "common.h"

#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <numeric>
#include <unistd.h>
#include <variant>
#include <vector>

namespace Database::Storing {

std::variant<Pager*, Errors::Error> Pager::OpenPager(const std::string& filepath)
{
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if (!File::RecognizedDatabaseSignature(fd)) {
        File::AddDatabaseSignature(fd);
    }

    if (fd == -1) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("File was not found with name {}", filepath), 0, 0, Errors::ERROR_FILE_NOT_FOUND);
    }

    off_t filelength = lseek(fd, 0, SEEK_END);

    return new Pager(fd, filelength);
}

std::variant<void*, Errors::Error> Pager::GetPage(uint32_t pageNumber)
{

    if (pageNumber > MAX_PAGES) {

        return Errors::Error(Errors::ErrorType::RuntimeError, "Tried to access unowned memory", 0, 0, Errors::ERROR_WRONG_MEMORY_ACCESS);
    }

    if (m_Pages[pageNumber] == nullptr) {
        void* page = malloc(DEFAULT_PAGE_SIZE);

        m_Pages[pageNumber] = page;
    }

    return m_Pages[pageNumber];
}

int File::GetTableCount(int fd)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    read(fd, &buffer, DB_INT8_SIZE);

    return buffer;
}

void File::SetTableCount(int fd, int value)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    buffer = value;

    write(fd, &buffer, DB_INT8_SIZE);
}

void File::IncrTableCount(int fd)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    read(fd, &buffer, DB_INT8_SIZE);

    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    buffer++;

    write(fd, &buffer, DB_INT8_SIZE);
}

bool File::RecognizedDatabaseSignature(int fd)
{
    char buf[52];

    int e = read(fd, buf, 52 * DB_CHAR_SIZE);

    for (uint8_t i = 0; i < 52; i++) {

        if (buf[i] != signature[i]) {
            return false;
        }
    }
    return true;
}

void File::AddDatabaseSignature(int fd)
{
    int e = write(fd, signature, 52 * DB_CHAR_SIZE);
}

} // namespace Database::Storing
