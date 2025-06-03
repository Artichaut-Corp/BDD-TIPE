#ifndef TABLE_H

#define TABLE_H

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include "column.h"
#include "types.h"

namespace Database::Storing {

//  Contient les informations immuables liées à une table
class TableInfo {
    DbInt m_IsSys;

    DbInt m_CurrentMaxRecord;

    DbInt m_ColumnNumber;

    uint64_t m_FirstColumnOffset;

public:
    std::unordered_map<std::string, ColumnInfo> m_Columns;

    TableInfo() = default;

    TableInfo(bool sys, int current_max_record, int column_number,
        std::vector<std::pair<std::string, ColumnInfo>> columns)
        : m_IsSys(sys)
        , m_CurrentMaxRecord(current_max_record)
        , m_ColumnNumber(column_number)
    {

        m_Columns.insert(columns.begin(), columns.end());
    }

    bool IsSys() { return m_IsSys; }

    ColumnInfo& GetColumnInfo(const std::string& column_name)
    {

        return m_Columns.at(column_name);
    }

    ColumnInfo operator[](const std::string& column_name)
    {
        return m_Columns.at(column_name);
    }

    // Etrange manière de le faire + pas très opti
    uint64_t GetColumnsFirstOffset()
    {
        uint64_t ret = std::pow(2, 64) - 1;

        for (auto iter = m_Columns.begin(); iter != m_Columns.end(); ++iter) {
            uint64_t p = iter->second.GetOffset();

            if (p < ret) {
                ret = p;
            }
        }

        return ret;
    }

    std::unordered_map<std::string, ColumnData> Map(const std::string& name,
        std::vector<DbInt> offsets)
    {
        return { { "name", Convert::StringToDbString(name) },
            { "offsets", Convert::VectorToDbIntArray(offsets) },
            { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentMaxRecord },
            { "column_number", m_ColumnNumber } };
    }

    std::unordered_map<std::string, ColumnData> Map(const std::string& name)
    {
        return { { "name", Convert::StringToDbString(name) },
            { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentMaxRecord },
            { "column_number", m_ColumnNumber } };
    }

    std::unordered_map<std::string, ColumnData> Map()
    {
        return { { "is_sys", m_IsSys },
            { "current_max_record", m_CurrentMaxRecord },
            { "column_number", m_ColumnNumber } };
    }
};

}

#endif // !TABLE_H
