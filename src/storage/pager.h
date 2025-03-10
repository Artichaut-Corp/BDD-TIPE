#include <array>
#include <cstdint>
#include <string>
#include <variant>

#include "../errors.h"

#ifndef PAGE_H

#define MAX_PAGES 4096
#define DEFAULT_PAGE_SIZE 4096

namespace Database::Storing {

class Pager {
private:
    int m_FileDescriptor;
    uint32_t m_FileLength;

    std::array<void*, MAX_PAGES> m_Pages;

public:
    Pager(int fd, uint32_t lenght)
        : m_FileDescriptor(fd)
        , m_FileLength(lenght)
    {
        for (uint32_t i = 0; i < MAX_PAGES; i++) {
            this->m_Pages[i] = nullptr;
        }
    }

    static std::variant<Pager*, Errors::Error> OpenPager(const std::string& filepath);

    std::variant<void*, Errors::Error> GetPage(uint32_t pageNumber);
};

}

#endif // !PAGE_H
