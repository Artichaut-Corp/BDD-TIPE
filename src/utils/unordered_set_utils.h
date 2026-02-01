#ifndef UNORDERED_SET_UTILS_H
#define UNORDERED_SET_UTILS_H

#include "data_process_system/namingsystem.h"

#include <unordered_set>

namespace Database::Utils {

inline bool is_subset(std::unordered_set<QueryPlanning::ColonneNamesSet*>* A, std::unordered_set<QueryPlanning::ColonneNamesSet*>* B)
{
    for (auto& elemC : *A) {
        bool est_trouve = false;
        for (auto& elemT : *B) {
            if ((*elemC) == (*elemT)) {
                est_trouve = true;
                break;
            }
        }
        if (!est_trouve) {
            return false;
        }
    }
    return true;
}

} // namespace Database::Utils

#endif //! UNORDERED_SET_UTILS_H
