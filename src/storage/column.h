#include <cstdint>
#include <string>
#include <unordered_map>

#include "types.h"

#ifndef COLUMN_H

#define COLUMN_H

namespace Database::Storing {

class ColumnInfo {

    DbInt64 m_Offset;

    DbInt8 m_ElementSize;

    DbInt m_CurrentMaxRecord;

    DbBool m_Sortable;

    DbBool m_IsSorted;

    DbBool m_Compressable;

    DbBool m_IsCompressed;

public:
    ColumnInfo() = default;

    ColumnInfo(int offset, uint8_t e_size, int current_max, bool opt)
        : m_Offset(offset)
        , m_ElementSize(e_size)
        , m_CurrentMaxRecord(current_max)
        , m_Sortable(opt)
        , m_IsSorted(opt)
        , m_Compressable(opt)
        , m_IsCompressed(opt)
    {
    }

    ColumnInfo(int offset, uint8_t e_size, int current_max, bool sortable,
        bool sorted, bool comrpessable, bool compressed)
        : m_Offset(offset)
        , m_ElementSize(e_size)
        , m_CurrentMaxRecord(current_max)
        , m_Sortable(sortable)
        , m_IsSorted(sorted)
        , m_Compressable(compressed)
        , m_IsCompressed(compressed)
    {
    }

    uint64_t GetOffset() const { return m_Offset; }

    uint8_t GetElementSize() const { return m_ElementSize; }

    int GetCurrentMax() const { return m_CurrentMaxRecord; }

    std::unordered_map<std::string, ColumnData> Map(const std::string& name)
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "name", Convert::StringToDbString(name) },
            { "offset", m_Offset },
            { "element_size", m_ElementSize },
            { "current_max_record", m_CurrentMaxRecord },
            { "sortable", m_Sortable },
            { "sorted", m_IsSorted },
            { "compressable", m_Compressable },
            { "compressed", m_IsCompressed }
        };

        return res;
    }

    std::unordered_map<std::string, ColumnData> Map()
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "offset", m_Offset },
            { "element_size", m_ElementSize },
            { "current_max_record", m_CurrentMaxRecord },
            { "sortable", m_Sortable },
            { "sorted", m_IsSorted },
            { "compressable", m_Compressable },
            { "compressed", m_IsCompressed }
        };

        return res;
    }
};

} // namespace Database::Storing

#endif // !COLUMN_H
