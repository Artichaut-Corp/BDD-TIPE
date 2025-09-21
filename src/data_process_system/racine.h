#include "../algebrizer_types.h"

#include <vector>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    std::string NomColonne;
    std::vector<ColumnData*> data; // données immuables
public:
    Racine(std::string NomColonne_)
        : NomColonne(NomColonne_)
    {
        std::vector<ColumnData> ValeurRecuper; // = GetColonneValeurByName(NomColonne_)
    }

    ColumnData* getValue(const size_t idx) const 
    {
        return data[idx];
    }

    size_t size() const
    {
        return data.size();
    }
};

} // Database::QueryPlanning

#endif
