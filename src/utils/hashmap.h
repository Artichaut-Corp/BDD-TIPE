#include "../../lib/RobinHood/robin_hood.h"
#include "../storage/types.h"
#include "../algebrizer_types.h"
#include <cstdint>

#ifndef HASH_UTILS_H
#define HASH_UTILS_H

namespace Database::Utils::Hash {

}
namespace robin_hood {
template <>
struct hash<Database::ColumnData> {
    size_t operator()(const Database::ColumnData& c) const noexcept {
        return Database::QueryPlanning::hashColumn(c); // ok, on utilise la fonction de hash dans Utils::Hash
    }
};
}
namespace Database::Utils::Hash {

// --- Clé composite dynamique ---
struct MultiKeyDyn {
    std::vector<ColumnData> keys;

    bool operator==(const MultiKeyDyn& other) const noexcept
    {
        if (keys.size() != other.keys.size())
            return false;
        for (size_t i = 0; i < keys.size(); ++i)
            if (keys[i] != other.keys[i])
                return false;
        return true;
    }
};

// --- Hash combiné dynamique ---
struct MultiKeyDynHash {
    size_t operator()(const MultiKeyDyn& mk) const noexcept
    {
        uint64_t h = 0xcbf29ce484222325ULL;
        for (const auto& key : mk.keys) {
            uint64_t part = Database::QueryPlanning::hashColumn(key);
            h ^= Database::QueryPlanning::mix64(part + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
        }
        return static_cast<size_t>(h);
    }
};

using MultiValueMapDyn = std::unordered_map<MultiKeyDyn, robin_hood::unordered_set<Database::ColumnData>, MultiKeyDynHash>;

inline void addValue(MultiValueMapDyn& map, const MultiKeyDyn& key, const ColumnData& value)
{
    map[key].insert(value);
}
}

#endif // !HASH_UTILS_H
