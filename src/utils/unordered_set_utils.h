#ifndef UNORDERED_SET_UTILS_H
#define UNORDERED_SET_UTILS_H

#include <unordered_set>
#include "../data_process_system/namingsystem.h"
namespace Database::Utils {

inline bool is_subset(const std::unordered_set<QueryPlanning::ColonneNamesSet*>* A,const std::unordered_set<QueryPlanning::ColonneNamesSet*>* B)
{   
    for ( auto* elemC : *A) {
        bool est_trouve = false;
        for( auto* elemT:*B){
            if ((*elemC)==(*elemT)){
                est_trouve = true;
                break;
            }
        }
        if(!est_trouve){
            return false;
        }
    }
    return true;
}

} // namespace Database::Utils

#endif //!UNORDERED_SET_UTILS_H
