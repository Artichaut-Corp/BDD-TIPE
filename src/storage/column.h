#include <string>
#include <tuple>
#include <unordered_map>

#include "cursor.h"
#include "types.h"

#ifndef COLUMN_H

#define COLUMN_H

namespace Database::Storing {

class ColumnInfo {

    DbUInt m_Offset;

    // Not actually written in memory as we can deduce it from the type
    DbUInt8 m_ElementSize;

    DbElemType m_Type;

    DbBool m_Sortable = false;

    DbBool m_IsSorted = false;

    DbUInt64 m_SortedColumnOffset;

    DbBool m_Compressable = false;

    DbBool m_IsCompressed = false;

public:
    using ColumnInfoTuple = std::tuple<DbString, DbUInt, DbUInt8, DbBool, DbBool, DbUInt64, DbBool, DbBool>;

    ColumnInfo() = default;

    // Simplest contructor for unsorted and uncompressed
    ColumnInfo(DbUInt offset, DbElemType e_type, DbBool opt)
        : m_Offset(offset)
        , m_ElementSize(Convert::TypeToTypeSize(e_type))
        , m_Type(e_type)
        , m_Sortable(opt)
        , m_IsSorted(opt)
        , m_Compressable(opt)
        , m_IsCompressed(opt)
    {
    }

    // Contructor for indexed column
    ColumnInfo(DbUInt offset, DbElemType e_type, DbBool sortable, DbBool sorted, DbUInt64 sorted_offset)
        : m_Offset(offset)
        , m_ElementSize(Convert::TypeToTypeSize(e_type))
        , m_Type(e_type)
        , m_Sortable(sortable)
        , m_IsSorted(sorted)
        , m_SortedColumnOffset(sorted_offset)
    {
    }

    ColumnInfo(DbUInt offset, DbElemType e_type, DbBool sortable, DbBool sorted, DbUInt64 sorted_offset,
        DbBool compressable, DbBool compressed)
        : m_Offset(offset)
        , m_Type(e_type)
        , m_ElementSize(Convert::TypeToTypeSize(e_type))
        , m_Sortable(sortable)
        , m_IsSorted(sorted)
        , m_Compressable(compressable)
        , m_IsCompressed(compressed)
    {
    }

    DbUInt64 GetOffset() const { return m_Offset; }

    DbUInt64 GetIndexOffset() const { return m_SortedColumnOffset; }

    DbUInt8 GetElementSize() const { return m_ElementSize; }

    DbElemType GetType() const { return m_Type; }

    DbBool IsSorted() const { return m_IsSorted; }

    DbBool IsCompressed() const { return m_IsCompressed; }

    static std::unique_ptr<std::vector<ColumnInfoTuple>>
    GetColumnsData(int fd, int total_column_number);

    std::unordered_map<std::string, ColumnData> Map(const std::string& name)
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "name", Convert::StringToDbString(name) }, { "offset", m_Offset },
            { "type", static_cast<DbUInt8>(m_Type) }, { "sortable", m_Sortable },
            { "sorted", m_IsSorted }, { "index_offset", m_SortedColumnOffset }, { "compressable", m_Compressable },
            { "compressed", m_IsCompressed }
        };

        return res;
    }

    std::unordered_map<std::string, ColumnData> Map()
    {

        std::unordered_map<std::string, ColumnData> res = {
            { "offset", m_Offset },
            { "type", static_cast<DbUInt8>(m_Type) },
            { "sortable", m_Sortable },
            { "sorted", m_IsSorted },
            { "index_offset", m_SortedColumnOffset },
            { "compressable", m_Compressable },
            { "compressed", m_IsCompressed }
        };

        return res;
    }
};

} // namespace Database::Storing

#endif // !COLUMN_H
