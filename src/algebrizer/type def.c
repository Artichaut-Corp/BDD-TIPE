#include <cstdint>
#include <cstdio>
#include <iostream>
#include <optional>
#include <variant>

#include "type def.h"

namespace Database::Querying {

using DbNull = std::nullopt_t;
using DbInt = uint32_t;
using DbChar = uint8_t;
using DbString = std::array<uint8_t, 255>;

using ColumnData = std::variant<DbNull, DbInt, DbString>;


void afficherColumnData(const ColumnData& col)
{
    std::visit([](auto&& arg) {
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

std::string getColumnTypeName(const ColumnData& col)
{
    return std::visit([](auto&& value) -> std::string {
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
}

bool operator==(const ColumnData& larg, const ColumnData& rarg)
{
    return std::visit(
        [](auto&& l, auto&& r) -> bool {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, DbInt> && std::is_same_v<R, DbInt>)
                return l == r;

            if constexpr (std::is_same_v<L, DbNull> && std::is_same_v<R, DbNull>)
                return true; // égalité de null à null = true

            if constexpr (std::is_same_v<L, DbString> && std::is_same_v<R, DbString>)
                return l == r;

            std::cout << "Comparaison impossible entre " << getColumnTypeName(l) << " et " << getColumnTypeName(r) << "\n";
            return false;
        },
        larg, rarg);
}

bool operator!=(const ColumnData& larg, const ColumnData& rarg)
{
    return std::visit(
        [](auto&& l, auto&& r) -> bool {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, DbInt> && std::is_same_v<R, DbInt>)
                return l != r;

            if constexpr (std::is_same_v<L, DbNull> && std::is_same_v<R, DbNull>)
                return false; // null != null = false

            if constexpr (std::is_same_v<L, DbString> && std::is_same_v<R, DbString>)
                return l != r;

            std::cout << "Comparaison impossible entre " << getColumnTypeName(l) << " et " << getColumnTypeName(r) << "\n";
            return true;
        },
        larg, rarg);
}

bool operator<(const ColumnData& larg, const ColumnData& rarg)
{
    return std::visit(
        [](auto&& l, auto&& r) -> bool {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, DbInt> && std::is_same_v<R, DbInt>)
                return l < r;

            if constexpr (std::is_same_v<L, DbNull> || std::is_same_v<R, DbNull>) {
                std::cout << "Comparaison < non définie pour DbNull\n";
                return false;
            }

            if constexpr (std::is_same_v<L, DbString> && std::is_same_v<R, DbString>)
                return l < r;

            std::cout << "Comparaison impossible entre " << getColumnTypeName(l) << " et " << getColumnTypeName(r) << "\n";
            return false;
        },
        larg, rarg);
}

bool operator>(const ColumnData& larg, const ColumnData& rarg)
{
    return std::visit(
        [](auto&& l, auto&& r) -> bool {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, DbInt> && std::is_same_v<R, DbInt>)
                return l > r;

            if constexpr (std::is_same_v<L, DbNull> || std::is_same_v<R, DbNull>) {
                std::cout << "Comparaison > non définie pour DbNull\n";
                return false;
            }

            if constexpr (std::is_same_v<L, DbString> && std::is_same_v<R, DbString>)
                return l > r;

            std::cout << "Comparaison impossible entre " << getColumnTypeName(l) << " et " << getColumnTypeName(r) << "\n";
            return false;
        },
        larg, rarg);
}

bool operator<=(const ColumnData& larg, const ColumnData& rarg)
{
    return !(larg > rarg);
}

bool operator>=(const ColumnData& larg, const ColumnData& rarg)
{
    return !(larg < rarg);
}
};