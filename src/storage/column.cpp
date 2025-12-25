#include "column.h"
#include "common.h"

namespace Database::Storing {

std::unique_ptr<std::vector<
    ColumnInfo::ColumnInfoTuple>>
ColumnInfo::GetColumnsData(int fd, int total_column_number)
{

    auto res = std::make_unique<std::vector<
        ColumnInfo::ColumnInfoTuple>>();

    res->reserve(total_column_number);

    // initialise each container
    std::vector<DbString> name = {};
    std::vector<DbUInt> column_offset = {};
    std::vector<DbUInt8> type = {};
    std::vector<DbBool> is_sortable = {};
    std::vector<DbBool> is_sorted = {};
    std::vector<DbUInt64> sorted_offset = {};
    std::vector<DbBool> is_compressable = {};
    std::vector<DbBool> is_compressed = {};

    // Get the data
    DbUInt64 offset = SCHEMA_COLUMN_OFFSET;

    FileInterface::ReadVec(fd, name, &offset, DB_STRING_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_STRING_SIZE;

    FileInterface::ReadVec(fd, column_offset, &offset, DB_UINT_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_UINT_SIZE;

    FileInterface::ReadVec(fd, type, &offset, DB_UINT8_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_UINT8_SIZE;

    FileInterface::ReadVec(fd, is_sortable, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, is_sorted, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, sorted_offset, &offset, DB_UINT64_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_UINT64_SIZE;

    FileInterface::ReadVec(fd, is_compressable, &offset, DB_BOOL_SIZE, total_column_number);

    offset += (MAX_TABLE * MAX_COLUMN_PER_TABLE - total_column_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, is_compressed, &offset, DB_BOOL_SIZE, total_column_number);

    // Format it into res

    for (int i = 0; i < total_column_number; i++) {
        auto e = std::make_tuple(name[i], column_offset[i], type[i],
            is_sortable[i], is_sorted[i], sorted_offset[i], is_compressable[i],
            is_compressed[i]);

        res->emplace_back(e);
    }

    return res;
}

} // namespace Database::Storing
