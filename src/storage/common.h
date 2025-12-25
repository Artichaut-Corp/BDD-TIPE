#include <cstdio>
#include <span>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>

#include "../errors.h"
#include "types.h"

#ifndef FILE_COMMON_H

#define FILE_COMMON_H

namespace Database::Storing {

class FileInterface {
public:
    template <typename T>
    static void ReadField(int fd, T* buffer, DbUInt64* offset, DbUInt64 size)
    {
        lseek(fd, *offset, SEEK_SET);

        int bytes_read = read(fd, &buffer, size);

        *offset += bytes_read;
    }

    template <typename T>
    static void ReadVec(int fd, std::vector<T>& vec, DbUInt64* offset, const size_t element_size, int element_count)
    {
        vec.resize(element_count);

        if (lseek(fd, *offset, SEEK_SET) == -1) {
            throw std::runtime_error("Failed to seek");
        }

        size_t total_bytes = element_size * element_count;
        ssize_t bytes_read = read(fd, vec.data(), total_bytes);

        if (bytes_read > 0) {
            *offset += bytes_read;
        }

        if (static_cast<size_t>(bytes_read) != total_bytes) {
            // throw std::runtime_error("Failed to read the expected amount of data");
            std::cout << "Warning less bytes read than expected\n";
        }
    }

    /*
        template <typename T>
        static void ReadVec(int fd, std::vector<T>& vec, DbUInt64* offset,

            const DbInt8 element_size, int element_count)
        {
            vec.resize(element_count);

            ReadRange(fd, vec, offset, element_size, element_count);
        }

        template <std::ranges::contiguous_range R>
        static void ReadRange(int fd, R& range, DbUInt64* offset,
            const DbInt8 element_size, int element_count)
        {
            using T = std::ranges::range_value_t<R>;

            static_assert(std::is_trivially_copyable_v<T>);

            auto bytes = std::as_writable_bytes(std::span(range));

            lseek(fd, static_cast<off_t>(*offset), SEEK_CUR);

            ssize_t r = read(fd,
                bytes.data(),
                element_size * element_count);

            if (r <= 0) {
                throw std::runtime_error("pread failed");
            }

            *offset += element_size * element_count;
        }
      */
};

} // namespace Database::Storing

#endif // !FILE_COMMON_H
