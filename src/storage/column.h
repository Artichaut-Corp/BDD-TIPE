#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>

#include "types.h"

#ifndef COLUMN_H

#define COLUMN_H

namespace Database::Storing {

class ColumnInfo {

    DbInt m_Offset;

    DbInt8 m_ElementSize;

    DbBool m_Sortable = false;

    DbBool m_IsSorted = false;

    DbInt64 m_SortedColumnOffset;

    DbBool m_Compressable = false;

    DbBool m_IsCompressed = false;

public:
    using ColumnInfoTuple = std::tuple<DbString, DbInt, DbInt8, DbBool, DbBool, DbInt64, DbBool, DbBool>;

    ColumnInfo() = default;

    // Simplest contructor for unsorted and uncompressed
    ColumnInfo(DbInt offset, DbInt8 e_size, DbBool opt)
        : m_Offset(offset)
        , m_ElementSize(e_size)
        , m_Sortable(opt)
        , m_IsSorted(opt)
        , m_Compressable(opt)
        , m_IsCompressed(opt)
    {
    }

    // Contructor for indexed column
    ColumnInfo(DbInt offset, DbInt8 e_size, DbBool sortable, DbBool sorted, DbInt64 sorted_offset)
        : m_Offset(offset)
        , m_ElementSize(e_size)
        , m_Sortable(sortable)
        , m_IsSorted(sorted)
        , m_SortedColumnOffset(sorted_offset)
    {
    }

    ColumnInfo(DbInt offset, DbInt8 e_size, DbBool sortable, DbBool sorted, DbInt64 sorted_offset,
        DbBool compressable, DbBool compressed)
        : m_Offset(offset)
        , m_ElementSize(e_size)
        , m_Sortable(sortable)
        , m_IsSorted(sorted)
        , m_Compressable(compressable)
        , m_IsCompressed(compressed)
    {
    }

    DbInt64 GetOffset() const { return m_Offset; }

    DbInt64 GetIndexOffset() const { return m_SortedColumnOffset; }

    DbInt8 GetElementSize() const { return m_ElementSize; }

    DbBool IsSorted() const { return m_IsSorted; }

    DbBool IsCompressed() const { return m_IsCompressed; }

    static std::unique_ptr<std::vector<ColumnInfoTuple>>
    GetColumnsData(int fd, int total_column_number);

    std::unordered_map<std::string, ColumnData> Map(const std::string& name)
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "name", Convert::StringToDbString(name) }, { "offset", m_Offset },
            { "element_size", m_ElementSize }, { "sortable", m_Sortable },
            { "sorted", m_IsSorted }, { "compressable", m_Compressable },
            { "compressed", m_IsCompressed }
        };

        return res;
    }

    std::unordered_map<std::string, ColumnData> Map()
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "offset", m_Offset },
            { "element_size", m_ElementSize },
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
