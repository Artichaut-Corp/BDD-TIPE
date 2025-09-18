#include "../algebrizer_types.h"
#include <cstddef>
#include <memory>
#include <variant>
#include <vector>


#ifndef agreg_H

#define ag_H

namespace Database::QueryPlanning {
using namespace Database::Querying;

static std::vector<int>& Sum();
static std::vector<int>& Average();
static std::vector<int>& Count();
static std::vector<int>& Min();
static std::vector<int>& Max();

enum class AggrType {
    NOTHING_F,
    AVG_F,
    COUNT_F,
    MAX_F,
    MIN_F,
    SUM_F
};

class ReturnType {
private:
    std::string colonne;
    AggrType opération;
public:
    ReturnType(std::string colonne_,AggrType type_)
        : colonne(colonne_)
        , opération(type_)
    {
    }
    AggrType GetType(){return opération;};

    std::string GetColonne(){return colonne;};
};

} // Database::QueryPlanning

#endif // 
