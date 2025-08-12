#include <cstdint>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#ifndef FILE_COMMON_H

#define FILE_COMMON_H

namespace Database::Storing {

class FileInterface {
public:
    template <typename T>
    static void ReadField(int fd, T* buffer, int* offset, int size)
    {
        lseek(fd, *offset, SEEK_SET);

        int bytes_read = read(fd, &buffer, size);

        *offset += bytes_read;
    }

    template <typename T>
    static void ReadVec(int fd, std::vector<T>& vec, int* offset,
        const uint8_t element_size, int element_count)
    {
        lseek(fd, *offset, SEEK_SET);

        T buffer;

        for (int i = 0; i < element_count; i++) {
            int bytes_read = read(fd, &buffer, element_size);

            *offset += bytes_read;

            if (bytes_read != element_size) {
                throw std::runtime_error("failed to read");
            }

            vec.emplace_back(buffer);
        }
    }
};

} // namespace Database::Storing

#endif // !FILE_COMMON_H
