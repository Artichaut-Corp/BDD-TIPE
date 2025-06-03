#include "errors.h"
#include "storage/cursor.h"
#include "storage/file.h"
#include "storage/table.h"
#include "storage/types.h"

#include <optional>

#ifndef STORAGE_H

#define STORAGE_H

namespace Database::Storing {

class Store {
public:
    static std::variant<Column, Errors::Error> GetColumn(int fd, const std::string& table_name, const std::string& column_name);

    template <typename R>
    static std::optional<Errors::Error> SetRecord(int fd, const std::string& table_name, R* record);
};

class CountryRecord {
    DbString name;
    DbInt pop;

    std::unordered_map<std::string, ColumnData> Map()
    {
        return { { "name", name }, { "pop", pop } };
    }

    // Crée en mémoire la représentation de la table pays
    static void CreateCountryTable(int fd, DBTableIndex& index)
    {
        // -- Country Table --
        // Text name
        // Int pop

        std::vector<DbInt>* offsets = new std::vector<DbInt>();

        // Passer comme offset la première localisation encore disponible, prob via
        // la class Cursor
        ColumnInfo* c_name = new ColumnInfo(
            Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE),
            DB_STRING_SIZE, 0, false);

        offsets->push_back(Record::Write(fd, index.at("schema_column"), c_name));

        ColumnInfo* c_pop = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
            DB_INT_SIZE, 0, false);

        offsets->push_back(Record::Write(fd, index.at("schema_column"), c_pop));

        auto c_columns = std::vector<std::pair<std::string, ColumnInfo>> {
            { "name", *c_name }, { "pop", *c_pop }
        };

        // 3. Write it on disk in the dedicated system table

        auto t = new TableInfo(false, 0, 2, c_columns);

        index.insert({ "country", *t });

        Record::Write(fd, index.at("schema_table"), t, "country", *offsets);
    }
};

} // namespace Database::Storing

#endif // !STORAGE_H
