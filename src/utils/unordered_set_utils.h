#ifndef UNORDERED_SET_UTILS_H
#define UNORDERED_SET_UTILS_H

#include <unordered_set>

namespace Database::Utils {

template <typename T>
inline bool is_subset(const std::unordered_set<T>* A, const std::unordered_set<T>* B) {
    for (const auto& elem : *A) {
        if (B->find(elem) == B->end()) {
            return false;
        }
    }
    return true;
}

} // namespace Database::Utils

#endif // UNORDERED_SET_UTILS_H
