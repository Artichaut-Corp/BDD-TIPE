#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <assert.h>

#ifndef DB_TYPES_H

#define DB_TYPES_H

namespace Database {

#define MAX_DBINT 0xFFFFFFFF
#define MAX_TABLE 64
#define MAX_COLUMN_PER_TABLE 16
#define MAX_ELEMENT_PER_COLUMN 4096
#define MAX_ARRAY_SIZE 16
#define MAX_STRING_LENGTH 255

constexpr uint8_t DB_BOOL_SIZE = 1; // 8 bits
constexpr uint8_t DB_INT8_SIZE = 1; // 8 bits
constexpr uint8_t DB_UINT8_SIZE = 1; // 8 bits
constexpr uint8_t DB_INT16_SIZE = 2; // 16 bits
constexpr uint8_t DB_UINT16_SIZE = 2; // 16 bits
constexpr uint8_t DB_INT_SIZE = 4; // 32 bits
constexpr uint8_t DB_UINT_SIZE = 4; // 32 bits
constexpr uint8_t DB_INT64_SIZE = 8; // 64 bits
constexpr uint8_t DB_UINT64_SIZE = 8; // 64 bits
constexpr uint8_t DB_FLOAT_SIZE = 4;
constexpr uint8_t DB_FLOAT64_SIZE = 8;
constexpr uint8_t DB_CHAR_SIZE = 1; // 8 bits
constexpr uint8_t DB_STRING_SIZE = MAX_STRING_LENGTH * DB_CHAR_SIZE; // 256 chars
constexpr uint8_t DB_INT_ARRAY_SIZE = DB_INT_SIZE * MAX_ARRAY_SIZE; // 16 int

constexpr uint8_t TREE_ORDER = 10;

#if MAX_ELEMENT_PER_COLUMN > MAX_DBINT
#define DB_COL_ELMT_INT_SIZE DB_UINT64_SIZE
using DbKey = uint64_t;
#else
#define DB_COL_ELMT_INT_SIZE DB_UINT32_SIZE
using DbKey = uint32_t;
#endif

// Taille d'un élément de la table système contenant les
// méta-données des tables
constexpr uint32_t DB_SCHEMA_TABLE_SIZE = DB_STRING_SIZE + DB_BOOL_SIZE + DB_UINT16_SIZE + DB_UINT8_SIZE + DB_UINT_SIZE;

// Taille d'un élément de la table système contenant les
// méta-données des colonnes
constexpr uint32_t DB_SCHEMA_COLUMN_SIZE = DB_STRING_SIZE + DB_UINT_SIZE + DB_UINT8_SIZE + 4 * DB_BOOL_SIZE;

constexpr uint32_t INDEX_HEADER_SIZE = 0;

constexpr uint32_t NODE_SIZE = DB_UINT64_SIZE + 2 * DB_BOOL_SIZE + 2 * DB_UINT8_SIZE + TREE_ORDER * DB_UINT64_SIZE * 2;

constexpr uint32_t MAX_NODE = MAX_ELEMENT_PER_COLUMN * NODE_SIZE;

constexpr uint32_t DB_INDEXED_REPR_SIZE = INDEX_HEADER_SIZE + MAX_NODE;

#define HEADER_OFFSET 0
#define SIGNATURE_OFFSET 0
#define TABLE_NUMBER_OFFSET 52 * DB_CHAR_SIZE
#define LAST_OFFSET_OFFSET 52 * DB_CHAR_SIZE + 1

#define SCHEMA_TABLE_OFFSET 52 * DB_CHAR_SIZE + 2

#define SCHEMA_COLUMN_OFFSET \
    SCHEMA_TABLE_OFFSET + MAX_TABLE* DB_SCHEMA_TABLE_SIZE
#define HEADER_END \
    SCHEMA_COLUMN_OFFSET + MAX_TABLE* MAX_COLUMN_PER_TABLE* DB_SCHEMA_COLUMN_SIZE

enum class DbElemType {
    DbNull = 0,
    DbBool,
    DbInt8,
    DbUInt8,
    DbInt16,
    DbUInt16,
    DbInt,
    DbUInt,
    DbInt64,
    DbUInt64,
    DbFloat,
    DbFloat64,
    DbChar,
    DbString,
};

using DbNull = std::nullopt_t;

using DbElemTypeSize = uint8_t;

using DbBool = uint8_t;
using DbInt8 = int8_t;
using DbUInt8 = uint8_t;
using DbInt16 = int16_t;
using DbUInt16 = uint16_t;
using DbInt = int32_t;
using DbUInt = uint32_t;
using DbInt64 = int64_t;
using DbUInt64 = uint64_t;
using DbFloat = float;
using DbFloat64 = double;
using DbChar = uint8_t;
using DbString = std::array<DbChar, MAX_STRING_LENGTH>;
using DbUIntArray = std::array<DbUInt, MAX_ARRAY_SIZE>;

static uint64_t m_CurrOffset = 0;

using ColumnData = std::variant<DbInt8, DbUInt8, DbInt16, DbUInt16, DbInt, DbUInt, DbInt64, DbUInt64, DbFloat, DbFloat64, DbString>;

using Column = std::variant<
    std::unique_ptr<std::vector<DbUInt8>>,
    std::unique_ptr<std::vector<DbInt8>>,
    std::unique_ptr<std::vector<DbUInt16>>,
    std::unique_ptr<std::vector<DbInt16>>,
    std::unique_ptr<std::vector<DbUInt>>,
    std::unique_ptr<std::vector<DbInt>>,
    std::unique_ptr<std::vector<DbUInt64>>,
    std::unique_ptr<std::vector<DbInt64>>,
    std::unique_ptr<std::vector<DbFloat>>,
    std::unique_ptr<std::vector<DbFloat64>>,
    std::unique_ptr<std::vector<DbString>>>;

using IndexedColumn = std::pair<
    std::variant<
        std::unique_ptr<std::vector<DbUInt8>>,
        std::unique_ptr<std::vector<DbUInt16>>,
        std::unique_ptr<std::vector<DbUInt>>,
        std::unique_ptr<std::vector<DbUInt64>>>,
    std::unique_ptr<std::vector<DbString>>>;

class Convert {
public:
    static DbUIntArray VectorToDbIntArray(std::vector<DbInt> v)
    {
        int l = v.size();

        DbUIntArray* r = new std::array<DbUInt, MAX_ARRAY_SIZE>();

        assert(l <= MAX_ARRAY_SIZE);

        for (int i = 0; i < l; i++) {
            r->at(i) = v[i];
        }

        return *r;
    }

    static std::string DbStringToString(const DbString& s)
    {
        int i = 0;
        std::string r;
        while (i < s.size() && s[i] != 0) {
            r.push_back(static_cast<char>(s[i]));
            ++i;
        }
        return r;
    }

    static DbString StringToDbString(const std::string& s)
    {
        DbString r {}; // value-initialize => tous les octets à 0
        int max_copy = std::min(s.size(), r.size() - 1); // garder place pour '\0' si nécessaire

        for (int i = 0; i < max_copy; ++i) {
            r[i] = static_cast<uint8_t>(s[i]);
        }
        // s'il reste de l'espace, r[max_copy] est déjà 0 grâce à l'initialisation
        return r;
    }

    static ColumnData intToColumnData(int value)
    {
        if (value <= std::numeric_limits<DbInt8>::max())
            return static_cast<DbInt8>(value);
        if (value <= std::numeric_limits<DbInt16>::max())
            return static_cast<DbInt16>(value);
        return static_cast<DbInt>(value);
    }

    static DbInt8 TypeToTypeSize(DbElemType type)
    {
        switch (type) {
        case DbElemType::DbNull:
            return 0;
        case DbElemType::DbBool:
            return DB_BOOL_SIZE;
        case DbElemType::DbInt8:
            return DB_INT8_SIZE;
        case DbElemType::DbUInt8:
            return DB_UINT8_SIZE;
        case DbElemType::DbInt16:
            return DB_INT16_SIZE;
        case DbElemType::DbUInt16:
            return DB_UINT16_SIZE;
        case DbElemType::DbInt:
            return DB_INT_SIZE;
        case DbElemType::DbUInt:
            return DB_UINT_SIZE;
        case DbElemType::DbInt64:
            return DB_INT64_SIZE;
        case DbElemType::DbUInt64:
            return DB_UINT64_SIZE;
        case DbElemType::DbFloat:
            return DB_FLOAT_SIZE;
        case DbElemType::DbFloat64:
            return DB_FLOAT64_SIZE;
        case DbElemType::DbChar:
            return DB_CHAR_SIZE;
        case DbElemType::DbString:
            return DB_STRING_SIZE;
        }

        return 0;
    }
};
inline std::ostream& operator<<(std::ostream& out, const ColumnData& cd)
{
    std::visit([&](auto&& elem) {
        using T = std::decay_t<decltype(elem)>;
        if constexpr (std::is_same_v<T, DbString>) {
            out << Convert::DbStringToString(elem);
        } else {
            out << std::to_string(elem);
        }
    },
        cd);
    return out;
}
}
#endif // !TYPES_H
