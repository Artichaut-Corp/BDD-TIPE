#include <string>
#include <vector>
#include "../algebrizer/type def.h"
// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    std::vector<Database::Querying::ColumnData*> data; // données immuables
public:
    Racine(const std::vector<Database::Querying::ColumnData*> data_)
        : data(data_)
    {
    }

    Database::Querying::ColumnData getValue(const size_t idx) const // à modifier
        {
            // return data[idx]&   à définir en fonction de l'implémentaiton
            return Database::Querying::DbInt {42}; //à modifier évidemment
        }

    size_t size() const
    {
        return data.size();
    }

};