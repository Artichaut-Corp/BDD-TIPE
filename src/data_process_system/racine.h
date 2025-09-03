#include "../algebrizer_types.h"

#include <vector>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    std::vector<ColumnData*> data; // données immuables
public:
    Racine(const std::vector<ColumnData*> data_)
        : data(data_)
    {
    }

    ColumnData getValue(const size_t idx) const // à modifier
    {
        // return data[idx]&   à définir en fonction de l'implémentaiton
        return DbInt { 42 }; // à modifier évidemment
    }

    size_t size() const
    {
        return data.size();
    }
};

} // Database::QueryPlanning

#endif
