#include "storage.h"

namespace Database::Storing {

std::variant<Column, Errors::Error> Store::DB_GetColumn(int fd, DBTableIndex* Index, const std::string& table_name, const std::string& column_name)
{

    TableInfo* t;

    try {
        t = &Index->at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table '{}' does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    if (t->GetElementNumber() == 0) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table '{}' is empty", table_name), 0, 0, Errors::ERROR_TABLE_EMPTY);
    }

    ColumnInfo c;

    try {
        c = t->GetColumnInfo(column_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Column '{}' does not exist in table '{}'", column_name, table_name), 0, 0, Errors::ERROR_COLUMN_DOES_NOT_EXIST);
    }

    switch (c.GetElementSize()) {
    case 1:
        return Record::GetColumn<DbInt8>(fd, c, t->GetElementNumber());
    case 2:
        return Record::GetColumn<DbInt16>(fd, c, t->GetElementNumber());
    case 4:
        return Record::GetColumn<DbInt>(fd, c, t->GetElementNumber());
    case 8:
        return Record::GetColumn<DbInt64>(fd, c, t->GetElementNumber());
    case 255:
        return Record::GetColumn<DbString>(fd, c, t->GetElementNumber());
    default:
        throw std::runtime_error("Unrecognized element size inside ColumnInfo.");
    }
}

template <typename R>
std::optional<Errors::Error> Store::DB_SetRecord(int fd, DBTableIndex* Index, const std::string& table_name, R* record)
{
    TableInfo* t;

    try {
        t = &Index->at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table '{}' does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    Record::Write(fd, t, record);

    return std::nullopt;
}

std::optional<Errors::Error> Store::DB_SetData(int fd, DBTableIndex* Index, const std::string& table_name, const std::unordered_map<std::string, ColumnData>& data)
{
    TableInfo* t;

    try {
        t = &Index->at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table '{}' does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    Record::Write(fd, t, data);

    return std::nullopt;
}

}
