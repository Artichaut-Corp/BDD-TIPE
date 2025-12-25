#include <cstdio>
#include <iostream>
#include <string>

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
    int i = lhs.index();

    if (lhs.index() == rhs.index()) {
        switch (lhs.index()) {
        case 0:
            return std::get<0>(lhs) == std::get<0>(rhs);
        case 1:
            return std::get<1>(lhs) == std::get<1>(rhs);
        case 2:
            return std::get<2>(lhs) == std::get<2>(rhs);
        case 3:
            return std::get<3>(lhs) == std::get<3>(rhs);
        case 4:
            return std::get<4>(lhs) == std::get<4>(rhs);
        case 5:
            return std::get<5>(lhs) == std::get<5>(rhs);
        case 6:
            return std::get<6>(lhs) == std::get<6>(rhs);
        case 7:
            return std::get<7>(lhs) == std::get<7>(rhs);
        case 8:
            return std::get<8>(lhs) == std::get<8>(rhs);
        case 9:
            return std::get<9>(lhs) == std::get<9>(rhs);
        case 10:
            return to_string_view(std::get<10>(lhs)) == to_string_view(std::get<10>(rhs)); // DbString
        }
    }

    // Si deux entiers différents → comparer via uint64_t
    auto is_integer = [](int idx) { return idx <= 9; };
    if (is_integer(lhs.index()) && is_integer(rhs.index())) {
        auto to_u64 = [](const ColumnData& c) -> uint64_t {
            switch (c.index()) {
            case 0:
                return std::get<0>(c);
            case 1:
                return std::get<1>(c);
            case 2:
                return std::get<2>(c);
            case 4:
                return std::get<4>(c);
            case 5:
                return std::get<5>(c);
            case 6:
                return std::get<6>(c);
            case 7:
                return std::get<7>(c);
            case 8:
                return std::get<8>(c);
            case 9:
                return std::get<9>(c);

            default:
                return 0;
            }
        };
        return to_u64(lhs) == to_u64(rhs);
    }

    return false; // types incompatibles
}

// Less than
inline bool column_less(const ColumnData& lhs, const ColumnData& rhs)
{
    if (lhs.index() == rhs.index()) {
        switch (lhs.index()) {
        case 0:
            return std::get<0>(lhs) < std::get<0>(rhs);
        case 1:
            return std::get<1>(lhs) < std::get<1>(rhs);
        case 2:
            return std::get<2>(lhs) < std::get<2>(rhs);
        case 3:
            return std::get<3>(lhs) < std::get<3>(rhs);
        case 4:
            return std::get<4>(lhs) < std::get<4>(rhs);
        case 5:
            return std::get<5>(lhs) < std::get<5>(rhs);
        case 6:
            return std::get<6>(lhs) < std::get<6>(rhs);
        case 7:
            return std::get<7>(lhs) < std::get<7>(rhs);
        case 8:
            return std::get<8>(lhs) < std::get<8>(rhs);
        case 9:
            return std::get<9>(lhs) < std::get<9>(rhs);
        case 10:
            return to_string_view(std::get<10>(lhs)) < to_string_view(std::get<10>(rhs)); // DbString
        }
    }

    // Si deux entiers différents
    auto is_integer = [](int idx) { return idx <= 3; };
    if (is_integer(lhs.index()) && is_integer(rhs.index())) {
        auto to_u64 = [](const ColumnData& c) -> uint64_t {
            switch (c.index()) {
            case 0:
                return std::get<0>(c);
            case 1:
                return std::get<1>(c);
            case 2:
                return std::get<2>(c);
            case 3:
                return std::get<3>(c);
            default:
                return 0;
            }
        };
        return to_u64(lhs) < to_u64(rhs);
    }

    // ordre arbitraire pour types différents
    return lhs.index() < rhs.index();
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
}
#endif // !ALGEBRIZER_TYPES_H
