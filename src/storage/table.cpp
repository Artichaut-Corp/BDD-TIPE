#include "table.h"

namespace Database::Storing {

std::unique_ptr<std::vector<std::pair<std::string, ColumnInfo>>>
TableInfo::GetTableColumns(
    int fd, uint8_t column_number, int column_number_beginning,
    const std::unique_ptr<std::vector<
        std::tuple<DbString, DbInt, DbInt8, DbBool, DbBool, DbBool, DbBool>>>& data)
{

    std::unique_ptr<std::vector<std::pair<std::string, ColumnInfo>>> res = std::make_unique<std::vector<std::pair<std::string, ColumnInfo>>>();

    res->reserve(column_number);

    for (int i = column_number_beginning;
        i < column_number_beginning + column_number; i++) {

        const auto [name, offset, size, sortable, sorted, compressable,
            compressed]
            = data->at(i);

        std::pair<std::string, ColumnInfo> e = {
            Convert::DbStringToString(name),
            ColumnInfo(offset, size, sortable, sorted, compressable, compressed)
        };

        res->emplace_back(e);
    }
    return res;
}

} // namespace Database::Storing
