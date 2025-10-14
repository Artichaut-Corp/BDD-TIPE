#include "../../lib/tsl/robin_map.h"
#include "../algebrizer_types.h"
#include "../storage/types.h"
#include <cstdint>
#include <set>

#ifndef HASH_UTILS_H
#define HASH_UTILS_H

template <>
struct std::hash<Database::ColumnData> {
    size_t operator()(const Database::ColumnData& c) const noexcept
    {
        return Database::QueryPlanning::hashColumn(c); // ok, on utilise la fonction de hash dans Utils::Hash
    }
};

namespace Database::Utils::Hash {

// --- Clé composite dynamique ---
class MultiKeyDyn {
public:
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

using MultiValueMapDyn = tsl::robin_pg_map<MultiKeyDyn, std::vector<std::set<Database::ColumnData>>, MultiKeyDynHash>;

inline void addValue(MultiValueMapDyn& map, const MultiKeyDyn& key, const ColumnData& value, const int& pos)
{
    auto& vec = map[key]; // crée une entrée vide si nécessaire
    if ((int)vec.size() <= pos)
        vec.resize(pos + 1); // agrandit avec des ensembles vides
    vec[pos].insert(value);
}
}

#endif // !HASH_UTILS_H
