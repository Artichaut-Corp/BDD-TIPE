#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <variant>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../errors.h"

#include "column.h"
#include "cursor.h"
#include "types.h"

#ifndef FILE_H

#define FILE_H

#define MAX_PAGES 4096
#define DEFAULT_PAGE_SIZE 4096

namespace Database::Storing {

constexpr uint8_t signature[52] = {
    0x64, 0x65, 0x20, 0x63, 0x68, 0x61, 0x63, 0x75, 0x6E, 0x20, 0x73,
    0x65, 0x6C, 0x6F, 0x6E, 0x20, 0x73, 0x65, 0x73, 0x20, 0x6D, 0x6F,
    0x79, 0x65, 0x6E, 0x73, 0x20, 0x61, 0x20, 0x63, 0x68, 0x61, 0x63,
    0x75, 0x6E, 0x20, 0x73, 0x65, 0x6C, 0x6F, 0x6E, 0x20, 0x73, 0x65,
    0x73, 0x20, 0x62, 0x65, 0x73, 0x6F, 0x69, 0x6E
};

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

    ~Pager()
    {
        close(m_FileDescriptor);
    }

    static std::variant<Pager*, Errors::Error> OpenPager(const std::string& filepath);

    std::variant<void*, Errors::Error> GetPage(uint32_t pageNumber);
};

class File {

private:
    int m_Fd;

    int m_Size;

    std::unique_ptr<Cursor> m_Cursor;

public:
    File(const std::string& filepath)
    {
        int fd = open(filepath.c_str(), O_RDWR, S_IWUSR | S_IRUSR);

        if (!RecognizedDatabaseSignature(fd)) {
            AddDatabaseSignature(fd);
        }

        struct stat s;

        if (fstat(fd, &s) == -1) {
            throw std::runtime_error("failed fstat");
        }

        m_Size = s.st_size;

        m_Fd = fd;
    }

    static int GetTableCount(int fd);

    static void SetTableCount(int fd, int value);

    static void IncrTableCount(int fd);

    static bool
    RecognizedDatabaseSignature(int fd);

    static void AddDatabaseSignature(int fd);

    // A modifier pour assurer la validité de la variable
    int Fd() const { return m_Fd; }
};
}

#endif // !FILE_H
