#include "errors.h"

#include "storage/column.h"
#include "storage/common.h"
#include "storage/cursor.h"
#include "storage/file.h"
#include "storage/record.h"
#include "storage/table.h"
#include "storage/types.h"

#include <optional>
#include <string>
#include <unordered_map>

#ifndef STORAGE_H

#define STORAGE_H

namespace Database::Storing {

using DBTableIndex = std::unordered_map<std::string, Storing::TableInfo>;

using DBTableOrder = std::vector<std::string>;

class Store {

public:
    static std::variant<Column, Errors::Error> GetDBColumn(int fd, DBTableIndex* Index, const std::string& table_name, const std::string& column_name);

    template <typename R>
    [[nodiscard]] static std::optional<Errors::Error> SetRecord(int fd, DBTableIndex* Index, const std::string& table_name, R* record);

    [[nodiscard]] static std::optional<Errors::Error> SetData(int fd, DBTableIndex* Index, const std::string& table_name, const std::unordered_map<std::string, ColumnData>& data);
};

class CountryRecord {
    DbString m_Name;
    DbInt m_Pop;

public:
    CountryRecord(const std::string& name, int pop)
        : m_Name(Convert::StringToDbString(name))
        , m_Pop(pop)
    {
    }
    std::unordered_map<std::string, ColumnData> Map()
    {
        return { { "name", m_Name }, { "pop", m_Pop } };
    }
};

} // namespace Database::Storing

#endif // !STORAGE_H
