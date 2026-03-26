#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>
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
            } else if constexpr (std::is_same_v<T, DbString>) {
                // Interpréter comme une chaîne terminée par '\0'
                for (auto c : arg) {
                    if (c == 0)
                        break; // fin de chaîne
                    std::cout << static_cast<char>(c);
                }
            } else {
                std::cout << arg;
            }
        },
        col);
}

// Convert string array to string_view (up to first '\0')
inline std::string_view to_string_view(const DbString& arr)
{
    int len = 0;
    for (; len < arr.size(); ++len) {
        if (arr[len] == 0)
            break; // '\0'
    }
    return std::string_view(reinterpret_cast<const char*>(arr.data()), len);
}

inline bool column_equal(const ColumnData& lhs, const ColumnData& rhs)
{
    return std::visit([](const auto& a, const auto& b) -> bool {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (is_signed_numeric_v<A> && is_signed_numeric_v<B>) {
            return static_cast<DbFloat64>(a) == static_cast<DbFloat64>(b);
        } else if constexpr (is_unsigned_numeric_v<A> && is_unsigned_numeric_v<B>) {
            return static_cast<DbUInt64>(a) == static_cast<DbUInt64>(b);
        } else if constexpr (is_float_numeric_v<A> && is_float_numeric_v<B>) {
            return static_cast<DbFloat64>(a) == static_cast<DbFloat64>(b);
        } else if constexpr (std::is_same_v<A, DbString> && std::is_same_v<B, DbString>) {
            return a == b;
        } else if constexpr (std::is_same_v<A, DbBool> && std::is_same_v<B, DbBool>) {
            return a == b;
        } else {
            return false;
        }
    },
        lhs, rhs);
}

// Compare two ColumnData
/*
inline bool column_equal(const ColumnData& lhs, const ColumnData& rhs)
{
    auto is_integer = [](int idx) { return idx <= 3; };

    if (is_integer(lhs.index()) && is_integer(rhs.index())) {
        auto to_uint64 = [](const auto& v) -> uint64_t {
            return static_cast<uint64_t>(v);
        };

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
*/

// Less than
/*
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
    auto is_integer = [](int idx) { return (idx >= 2 && idx <= 6); };

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
*/

inline bool column_less(const ColumnData& lhs, const ColumnData& rhs)
{
    return std::visit([](const auto& a, const auto& b) -> bool {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (is_signed_numeric_v<A> && is_signed_numeric_v<B>) {
            return static_cast<DbInt64>(a) < static_cast<DbInt64>(b);
        } else if constexpr (is_unsigned_numeric_v<A> && is_unsigned_numeric_v<B>) {
            return static_cast<DbUInt64>(a) < static_cast<DbUInt64>(b);
        } else if constexpr (is_float_numeric_v<A> && is_float_numeric_v<B>) {
            return static_cast<DbFloat64>(a) < static_cast<DbFloat64>(b);
        } else if constexpr (std::is_same_v<A, DbString> && std::is_same_v<B, DbString>) {
            return a < b;
        } else {
            return false;
        }
    },
        lhs, rhs);
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
    return std::visit([](const auto& value) -> uint64_t {
        using T = std::decay_t<decltype(value)>;

        // Normalize all signed integers to int64_t
        if constexpr (
            std::is_same_v<T, DbInt8> || std::is_same_v<T, DbInt16> || std::is_same_v<T, DbInt> || std::is_same_v<T, DbInt64>) {
            return mix64(static_cast<int64_t>(value));
        }

        // Normalize all unsigned integers to uint64_t
        else if constexpr (
            std::is_same_v<T, DbUInt8> || std::is_same_v<T, DbUInt16> || std::is_same_v<T, DbUInt> || std::is_same_v<T, DbUInt64>) {
            return mix64(static_cast<uint64_t>(value));
        }

        // Floating point
        else if constexpr (
            std::is_same_v<T, DbFloat> || std::is_same_v<T, DbFloat64>) {
            uint64_t bits;
            static_assert(sizeof(bits) >= sizeof(value));
            memcpy(&bits, &value, sizeof(value));

            return mix64(bits);
        }

        // String (FNV-1a)
        else if constexpr (std::is_same_v<T, DbString>) {
            uint64_t h = 0xcbf29ce484222325ULL;
            for (uint8_t b : value) {
                if (b == 0)
                    break;
                h = (h ^ b) * 0x100000001b3ULL;
            }
            return mix64(h);
        }

        // Safety net (should never happen)
        else {
            static_assert(always_false_v<T>, "Unhandled type in ColumnData");
        }
    },
        c);
}

}
#endif // !ALGEBRIZER_TYPES_H
