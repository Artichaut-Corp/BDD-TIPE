#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "storage/types.h"

#ifndef AlGEBRIZER_TYPE_H

#define AlGEBRIZER_TYPE_H

namespace Database::QueryPlanning {

inline void afficherColumnData(const ColumnData& col)
{
    std::visit(
        [](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, DbNull>) {
                std::cout << "NULL";
            } else if constexpr (std::is_same_v<T, DbInt>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, DbString>) {
                // Interpréter comme une chaîne terminée par '\0'
                for (auto c : arg) {
                    if (c == 0)
                        break; // fin de chaîne
                    std::cout << static_cast<char>(c);
                }
            }
        },
        col);
}

inline std::string getColumnTypeName(const ColumnData& col)
{
    return std::visit(
        [](auto&& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, DbNull>) {
                return "DbNull";
            } else if constexpr (std::is_same_v<T, DbInt>) {
                return "DbInt";
            } else if constexpr (std::is_same_v<T, DbString>) {
                return "DbString";
            } else {
                return "Type inconnu";
            }
        },
        col);
} // Convert string array to string_view (up to first '\0')
inline std::string_view to_string_view(const DbString& arr)
{
    int len = 0;
    for (; len < arr.size(); ++len) {
        if (arr[len] == 0)
            break; // '\0'
    }
    return std::string_view(reinterpret_cast<const char*>(arr.data()), len);
}
// Compare two ColumnData
inline bool column_equal(const ColumnData& lhs, const ColumnData& rhs)
{
    auto is_integer = [](int idx) { return idx <= 3; };

    if (is_integer(lhs.index()) && is_integer(rhs.index())) {
        auto to_uint64 = [](const auto& v) -> uint64_t {
            return static_cast<uint64_t>(v);
        };

        uint64_t left_value;
        uint64_t right_value;

        // Extract lhs
        if (std::holds_alternative<DbInt8>(lhs))
            left_value = to_uint64(std::get<DbInt8>(lhs));
        else if (std::holds_alternative<DbInt16>(lhs))
            left_value = to_uint64(std::get<DbInt16>(lhs));
        else if (std::holds_alternative<DbInt>(lhs))
            left_value = to_uint64(std::get<DbInt>(lhs));
        else
            left_value = to_uint64(std::get<DbInt64>(lhs));

        // Extract rhs
        if (std::holds_alternative<DbInt8>(rhs))
            right_value = to_uint64(std::get<DbInt8>(rhs));
        else if (std::holds_alternative<DbInt16>(rhs))
            right_value = to_uint64(std::get<DbInt16>(rhs));
        else if (std::holds_alternative<DbInt>(rhs))
            right_value = to_uint64(std::get<DbInt>(rhs));
        else {
            right_value = to_uint64(std::get<DbInt64>(rhs));
        }

        return left_value == right_value;
    } else if (lhs.index() == 4 && rhs.index() == 4) {
        // handle floating point comparison, for example
        return std::get<DbString>(lhs) == std::get<DbString>(rhs);
    }

    throw std::runtime_error("Types incompatibles");
    return false;
}

// Less than
inline bool column_less(const ColumnData& lhs, const ColumnData& rhs)
{
    auto is_integer = [](int idx) { return idx <= 3; };

    if (is_integer(lhs.index()) && is_integer(rhs.index())) {
        auto to_uint64 = [](const auto& v) -> uint64_t {
            return static_cast<uint64_t>(v);
        };

        uint64_t left_value;
        uint64_t right_value;

        // Extract lhs
        if (std::holds_alternative<DbInt8>(lhs))
            left_value = to_uint64(std::get<DbInt8>(lhs));
        else if (std::holds_alternative<DbInt16>(lhs))
            left_value = to_uint64(std::get<DbInt16>(lhs));
        else if (std::holds_alternative<DbInt>(lhs))
            left_value = to_uint64(std::get<DbInt>(lhs));
        else
            left_value = to_uint64(std::get<DbInt64>(lhs));

        // Extract rhs
        if (std::holds_alternative<DbInt8>(rhs))
            right_value = to_uint64(std::get<DbInt8>(rhs));
        else if (std::holds_alternative<DbInt16>(rhs))
            right_value = to_uint64(std::get<DbInt16>(rhs));
        else if (std::holds_alternative<DbInt>(rhs))
            right_value = to_uint64(std::get<DbInt>(rhs));
        else {
            right_value = to_uint64(std::get<DbInt64>(rhs));
        }

        return left_value < right_value;
    } else if (lhs.index() == 4 && rhs.index() == 4) {
        // handle floating point comparison, for example
        return std::get<DbString>(lhs) < std::get<DbString>(rhs);
    }

    throw std::runtime_error("Types incompatibles");
    return false;
}

// Autres opérateurs dérivés
inline bool operator==(const ColumnData& lhs, const ColumnData& rhs)
{
    return column_equal(lhs, rhs);
}
inline bool operator!=(const ColumnData& lhs, const ColumnData& rhs) { return !column_equal(lhs, rhs); }
inline bool operator<(const ColumnData& lhs, const ColumnData& rhs) { return column_less(lhs, rhs); }
inline bool operator>(const ColumnData& lhs, const ColumnData& rhs) { return column_less(rhs, lhs); }
inline bool operator<=(const ColumnData& lhs, const ColumnData& rhs) { return !column_less(rhs, lhs); }
inline bool operator>=(const ColumnData& lhs, const ColumnData& rhs) { return !column_less(lhs, rhs); }

inline ColumnData operator+(const ColumnData& a, const ColumnData& b)
{
    return std::visit([](auto&& va, auto&& vb) -> ColumnData {
        using A = std::decay_t<decltype(va)>;
        using B = std::decay_t<decltype(vb)>;

        if constexpr (std::is_same_v<A, DbString> || std::is_same_v<B, DbString>) {
            throw std::runtime_error("Addition not defined for DbString");
        } else {
            using Common = std::common_type_t<A, B>;
            Common sum = static_cast<Common>(va) + static_cast<Common>(vb);

            if constexpr (std::is_same_v<Common, DbInt8>)
                return ColumnData(static_cast<DbInt8>(sum));
            else if constexpr (std::is_same_v<Common, DbInt16>)
                return ColumnData(static_cast<DbInt16>(sum));
            else if constexpr (std::is_same_v<Common, DbInt>)
                return ColumnData(static_cast<DbInt>(sum));
            else
                return ColumnData(static_cast<DbInt64>(sum));
        }
    },
        a, b);
}

inline ColumnData operator/(const ColumnData& a, const ColumnData& b)
{
    return std::visit([](auto&& va, auto&& vb) -> ColumnData {
        using A = std::decay_t<decltype(va)>;
        using B = std::decay_t<decltype(vb)>;

        if constexpr (std::is_same_v<A, DbString> || std::is_same_v<B, DbString>) {
            throw std::runtime_error("Division not defined for DbString");
        } else {
            using Common = std::common_type_t<A, B>;
            Common sum = static_cast<Common>(va) / static_cast<Common>(vb);

            if constexpr (std::is_same_v<Common, DbInt8>)
                return ColumnData(static_cast<DbInt8>(sum));
            else if constexpr (std::is_same_v<Common, DbInt16>)
                return ColumnData(static_cast<DbInt16>(sum));
            else if constexpr (std::is_same_v<Common, DbInt>)
                return ColumnData(static_cast<DbInt>(sum));
            else
                return ColumnData(static_cast<DbInt64>(sum));
        }
    },
        a, b);
}
inline uint64_t mix64(uint64_t x) noexcept
{
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x ^= (x >> 31);
    return x;
}

// --- Hash d'une colonne ---
inline uint64_t hashColumn(const Database::ColumnData& c) noexcept
{
    // All integers normalized to int64_t
    if (std::holds_alternative<DbInt8>(c))
        return mix64(static_cast<int64_t>(std::get<DbInt8>(c)));
    if (std::holds_alternative<DbInt16>(c))
        return mix64(static_cast<int64_t>(std::get<DbInt16>(c)));
    if (std::holds_alternative<DbInt>(c))
        return mix64(static_cast<int64_t>(std::get<DbInt>(c)));
    if (std::holds_alternative<DbInt64>(c))
        return mix64(std::get<DbInt64>(c));

    // Strings handled normally
    const auto& s = std::get<DbString>(c);
    uint64_t h = 0xcbf29ce484222325ULL;
    for (uint8_t b : s) {
        if (b == 0) break;
        h = (h ^ b) * 0x100000001b3ULL; // FNV-1a
    }
    return mix64(h);
}


}
#endif // !ALGEBRIZER_TYPES_H