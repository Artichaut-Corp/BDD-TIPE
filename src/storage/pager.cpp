#include "pager.h"
#include "../utils.h"

#include <cstdlib>
#include <fcntl.h>
#include <format>
#include <unistd.h>
#include <variant>

namespace Database::Storing {

std::variant<Pager*, Errors::Error> Pager::OpenPager(const std::string& filepath)
{
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if (!Utils::RecognizedDatabaseSignature(fd)) {
        Utils::AddDatabaseSignature(fd);
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

}
