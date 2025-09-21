
#include <string>
#include <vector>
#include "../data_process_system/table.h"
#ifndef agreg_H

#define agreg_H
namespace Database::QueryPlanning {

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
    ReturnType(std::string colonne_, AggrType type_)
        : colonne(colonne_)
        , opération(type_)
    {
    }
    AggrType GetType() { return opération; };

    std::string GetColonne() { return colonne; };
};

class Final {
private:
    std::vector<ReturnType> ColonneInfo;
public:
    Final(std::vector<ReturnType> ColonneInfo_)
    : ColonneInfo(ColonneInfo_)
    {
    };

    Table* AppliqueAgregate(Table*);

    void AfficheResultat(Table*);
};

} // Database::QueryPlanning

#endif // !agreg_H
