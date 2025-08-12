#include "storage.h"
#include "storage/record.h"

#include <format>
#include <stdexcept>
#include <utility>

namespace Database::Storing {

DBTableIndex Index = {};
DBTableOrder TableOrder = {};

std::variant<Column, Errors::Error> Store::GetDBColumn(int fd, const std::string& table_name, const std::string& column_name)
{

    TableInfo* t;

    try {
        t = &Index.at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    ColumnInfo c;

    try {
        c = t->GetColumnInfo(column_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Column {} does not exist", table_name), 0, 0, Errors::ERROR_COLUMN_DOES_NOT_EXIST);
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
std::optional<Errors::Error> Store::SetRecord(int fd, const std::string& table_name, R* record)
{
    TableInfo* t;

    try {
        t = &Index.at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    Record::Write(fd, t, record);

    return std::nullopt;
}

std::optional<Errors::Error> Store::SetData(int fd, const std::string& table_name, const std::unordered_map<std::string, ColumnData>& data)
{
    TableInfo* t;

    try {
        t = &Index.at(table_name);
    } catch (std::out_of_range) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", table_name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }

    Record::Write(fd, t, data);

    return std::nullopt;
}

template <typename R>
std::variant<std::unique_ptr<R>, Errors::Error> Record::GetRecordFromTableName(const std::string& name, std::unordered_map<std::string, ColumnData> data)
{

    // TODO: Prob ajouter un try catch pour signaler l'absence de colonne

    if (name == "schema_table") {
        // TODO
    } else if (name == "schema_table") {
        // TODO
    } else if (name == "country") {

        auto country_record = std::make_unique<CountryRecord>(std::get<DbString>(data.at("name")), std::get<DbInt>(data.at("pop")));

        return country_record;
    } else {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("Table {} does not exist", name), 0, 0, Errors::ERROR_TABLE_DOES_NOT_EXIST);
    }
}

// Crée en mémoire la représentation de la table pays
void CountryRecord::CreateCountryTable(int fd)
{
    // -- Country Table --
    // Text name
    // Int pop

    DbInt first_offset = Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE);

    // Passer comme offset la première localisation encore disponible
    ColumnInfo* c_name = new ColumnInfo(first_offset, DB_STRING_SIZE, false);

    Record::Write(fd, &Index.at("schema_column"), c_name, "name");

    ColumnInfo* c_pop = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
        DB_INT_SIZE, false);

    Record::Write(fd, &Index.at("schema_column"), c_pop, "pop");

    auto c_columns = std::vector<std::pair<std::string, ColumnInfo>> {
        { "name", *c_name }, { "pop", *c_pop }
    };

    // 3. Write it on disk in the dedicated system table

    auto country = new TableInfo(false, 2, first_offset, c_columns);

    Index.insert({ "country", *country });

    Record::Write(fd, &Index.at("schema_table"), country, "country");

    TableOrder.push_back("country");

    File::IncrTableCount(fd);
}

}
