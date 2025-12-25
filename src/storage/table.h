#ifndef TABLE_H

#define TABLE_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "column.h"
#include "types.h"

namespace Database::Storing {

//  Contient les informations immuables liées à une table
class TableInfo {
    DbBool m_IsSys;

    DbUInt16 m_CurrentElementNb;

    DbUInt8 m_ColumnNumber;

    DbUInt m_FirstColumnOffset;

public:
    std::unordered_map<std::string, ColumnInfo> m_Columns;

    TableInfo() = default;

    TableInfo(DbBool sys, DbUInt8 column_number, DbUInt first_offset,
        std::vector<std::pair<std::string, ColumnInfo>> columns)
        : m_IsSys(sys)
        , m_CurrentElementNb(0)
        , m_ColumnNumber(column_number)
        , m_FirstColumnOffset(first_offset)

    {

        m_Columns.insert(columns.begin(), columns.end());
    }

    TableInfo(DbBool sys, DbUInt16 current_element_number, DbUInt8 column_number,
        DbUInt first_offset,
        std::vector<std::pair<std::string, ColumnInfo>> columns)
        : m_IsSys(sys)
        , m_CurrentElementNb(current_element_number)
        , m_ColumnNumber(column_number)
        , m_FirstColumnOffset(first_offset)
    {

        m_Columns.insert(columns.begin(), columns.end());
    }

    DbUInt GetElementNumber() { return m_CurrentElementNb; }

    DbBool IsSys() { return m_IsSys; }

    DbUInt8 GetColumnNumber() { return m_ColumnNumber; }

    void IncrMaxRecord()
    {
        m_CurrentElementNb++;
    }

    ColumnInfo& GetColumnInfo(const std::string& column_name)
    {

        return m_Columns.at(column_name);
    }

    ColumnInfo operator[](const std::string& column_name)
    {
        return m_Columns.at(column_name);
    }

    // 1. Récupérer toutes les données des colonnes (chaque méta-donnée un vec).
    // Besoin nb total de colonnes
    // 2. Découper et créer les vecteurs qui contiennent les données selon les
    // tables
    static std::unique_ptr<std::vector<std::pair<std::string, ColumnInfo>>>
    GetTableColumns(
        int fd, uint8_t column_number, int column_number_beginning,
        const std::unique_ptr<std::vector<
            ColumnInfo::ColumnInfoTuple>>& data);

    std::unordered_map<std::string, ColumnData> Map(const std::string& name,
        DbUInt offset)
    {
        return { { "name", Convert::StringToDbString(name) },
            { "column_offset", offset },
            { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentElementNb },
            { "column_number", m_ColumnNumber } };
    }

    std::unordered_map<std::string, ColumnData> Map(const std::string& name)
    {
        return { { "name", Convert::StringToDbString(name) },
            { "column_offset", m_FirstColumnOffset },
            { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentElementNb },
            { "column_number", m_ColumnNumber } };
    }

    std::unordered_map<std::string, ColumnData> Map()
    {
        return { { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentElementNb },
            { "column_number", m_ColumnNumber } };
    }
};

}

#endif // !TABLE_H
