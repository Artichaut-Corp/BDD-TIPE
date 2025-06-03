#include "storage.h"

#include <format>
#include <stdexcept>

namespace Database::Storing {

std::variant<Column, Errors::Error> Store::GetColumn(int fd, const std::string& table_name, const std::string& column_name)
{

    TableInfo t;

    try {
        t = Index.at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    ColumnInfo c;

    try {
        c = t.GetColumnInfo(column_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    return Record::GetColumn(fd, c);
}

template <typename R>
std::optional<Errors::Error> Store::SetRecord(int fd, const std::string& table_name, R* record)
{
    TableInfo t;

    try {
        t = Index.at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    Record::Write(fd, t, record);

    return std::nullopt;
}

}
