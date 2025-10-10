#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <assert.h>

#ifndef DB_TYPES_H

#define DB_TYPES_H

namespace Database {

#define MAX_TABLE 64
#define MAX_COLUMN_PER_TABLE 16
#define MAX_ELEMENT_PER_COLUMN 4096
#define MAX_ARRAY_SIZE 16
#define MAX_STRING_LENGTH 255

constexpr uint8_t DB_BOOL_SIZE = 1; // 8 bits
constexpr uint8_t DB_INT8_SIZE = 1; // 8 bits
constexpr uint8_t DB_INT16_SIZE = 2; // 16 bits
constexpr uint8_t DB_INT_SIZE = 4; // 32 bits
constexpr uint8_t DB_INT64_SIZE = 8; // 64 bits
constexpr uint8_t DB_CHAR_SIZE = 1; // 8 bits
constexpr uint8_t DB_STRING_SIZE = MAX_STRING_LENGTH * DB_CHAR_SIZE; // 256 chars
constexpr uint8_t DB_INT_ARRAY_SIZE = DB_INT_SIZE * MAX_ARRAY_SIZE; // 16 int

// Taille d'un élément de la table système contenant les
// méta-données des tables
constexpr uint32_t DB_SCHEMA_TABLE_SIZE = DB_STRING_SIZE + DB_BOOL_SIZE + DB_INT16_SIZE + DB_INT8_SIZE + DB_INT_SIZE;

// Taille d'un élément de la table système contenant les
// méta-données des colonnes
constexpr uint32_t DB_SCHEMA_COLUMN_SIZE = DB_STRING_SIZE + DB_INT_SIZE + DB_INT8_SIZE + 4 * DB_BOOL_SIZE;

#define HEADER_OFFSET 0
#define SIGNATURE_OFFSET 0
#define TABLE_NUMBER_OFFSET 52 * DB_CHAR_SIZE
#define LAST_OFFSET_OFFSET 52 * DB_CHAR_SIZE + 1
#define SCHEMA_TABLE_OFFSET 52 * DB_CHAR_SIZE + 2
#define SCHEMA_COLUMN_OFFSET \
    SCHEMA_TABLE_OFFSET + MAX_TABLE* DB_SCHEMA_TABLE_SIZE
#define HEADER_END \
    SCHEMA_COLUMN_OFFSET + MAX_TABLE* MAX_COLUMN_PER_TABLE* DB_SCHEMA_COLUMN_SIZE

enum class Type {
    Null = 0,
    Int,
    Text,
};

using DbNull = std::nullopt_t;

using DbBool = uint8_t;
using DbInt8 = uint8_t;
using DbInt16 = uint16_t;
using DbInt = uint32_t;
using DbInt64 = uint64_t;
using DbChar = uint8_t;
using DbString = std::array<uint8_t, MAX_STRING_LENGTH>;
using DbIntArray = std::array<DbInt, MAX_ARRAY_SIZE>;

static uint64_t m_CurrOffset = 0;

using ColumnData = std::variant<DbInt8, DbInt16, DbInt, DbInt64, DbString>;

using Column = std::variant<
    std::unique_ptr<std::vector<DbInt8>>, std::unique_ptr<std::vector<DbInt16>>, std::unique_ptr<std::vector<DbInt>>, std::unique_ptr<std::vector<DbInt64>>, std::unique_ptr<std::vector<DbString>>>;

class Convert {
public:
    static DbIntArray VectorToDbIntArray(std::vector<DbInt> v)
    {
        int l = v.size();

        DbIntArray* r = new std::array<DbInt, MAX_ARRAY_SIZE>();

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
        return static_cast<DbInt64>(value);
    }
};
inline std::ostream& operator<<(std::ostream& out, const ColumnData& cd)
{
    std::visit([&](auto&& elem) {
        using T = std::decay_t<decltype(elem)>;
        if constexpr (std::is_same_v<T, DbString>) {
            out<<Convert::DbStringToString(elem);
        } else {
            out<< std::to_string(elem);
        }
    },
        cd);
    return out;
}
}
#endif // !TYPES_H
