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

    DbInt16 m_CurrentElementNb;

    DbInt8 m_ColumnNumber;

    DbInt m_FirstColumnOffset;

public:
    std::unordered_map<std::string, ColumnInfo> m_Columns;

    TableInfo() = default;

    TableInfo(bool sys, uint8_t column_number, uint32_t first_offset,
        std::vector<std::pair<std::string, ColumnInfo>> columns)
        : m_IsSys(sys)
        , m_CurrentElementNb(0)
        , m_ColumnNumber(column_number)
        , m_FirstColumnOffset(first_offset)

    {

        m_Columns.insert(columns.begin(), columns.end());
    }

    TableInfo(bool sys, uint16_t current_element_number, uint8_t column_number,
        uint32_t first_offset,
        std::vector<std::pair<std::string, ColumnInfo>> columns)
        : m_IsSys(sys)
        , m_CurrentElementNb(current_element_number)
        , m_ColumnNumber(column_number)
        , m_FirstColumnOffset(first_offset)
    {

        m_Columns.insert(columns.begin(), columns.end());
    }

    DbInt GetElementNumber() { return m_CurrentElementNb; }

    DbBool IsSys() { return m_IsSys; }

    DbInt8 GetColumnNumber() { return m_ColumnNumber; }

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
        DbInt offset)
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
