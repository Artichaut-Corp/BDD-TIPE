#include "column.h"
#include "common.h"

namespace Database::Storing {

std::unique_ptr<std::vector<
    std::tuple<DbString, DbInt, DbInt8, DbBool, DbBool, DbBool, DbBool>>>
ColumnInfo::GetColumnsData(int fd, int total_column_number)
{

    auto res = std::make_unique<std::vector<
        std::tuple<DbString, DbInt, DbInt8, DbBool, DbBool, DbBool, DbBool>>>();

    res->reserve(total_column_number);

    // initialise each container
    std::vector<DbString> name = {};
    std::vector<DbInt> column_offset = {};
    std::vector<DbInt8> element_size = {};
    std::vector<DbBool> is_sortable = {};
    std::vector<DbBool> is_sorted = {};
    std::vector<DbBool> is_compressable = {};
    std::vector<DbBool> is_compressed = {};

    // ...reserve()
    name.reserve(total_column_number);
    column_offset.reserve(total_column_number);
    element_size.reserve(total_column_number);
    is_sortable.reserve(total_column_number);
    is_sorted.reserve(total_column_number);
    is_compressable.reserve(total_column_number);
    is_compressed.reserve(total_column_number);

    // Get the data
    int offset = SCHEMA_COLUMN_OFFSET;

    FileInterface::ReadVec(fd, name, &offset, DB_STRING_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_STRING_SIZE;

    FileInterface::ReadVec(fd, column_offset, &offset, DB_INT_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_INT_SIZE;

    FileInterface::ReadVec(fd, element_size, &offset, DB_INT8_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_INT8_SIZE;

    FileInterface::ReadVec(fd, is_sortable, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, is_sorted, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, is_compressable, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, is_compressed, &offset, DB_BOOL_SIZE, total_column_number);

    // Format it into res

    for (int i = 0; i < total_column_number; i++) {
        auto e = std::make_tuple(name[i], column_offset[i], element_size[i],
            is_sortable[i], is_sorted[i], is_compressable[i],
            is_compressed[i]);

        res->emplace_back(e);
    }

    return res;
}

} // namespace Database::Storing
