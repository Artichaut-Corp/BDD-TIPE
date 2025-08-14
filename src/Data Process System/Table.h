#include <map>
#include <string>
#include <vector>

#include "../algebrizer/type def.h"
#include "Colonne.h"

class Table {
private:
    std::vector<Colonne*> data; // données immuables
    std::map<std::string, size_t> map; // permet de trouver la position d'une colonne à partir de son nom

public:
    Table(const std::vector<Colonne*>& data_, const std::vector<std::string>& nom_colonne) // noms et colonnes dans le bon ordre
        : data(data_)
    {
        for (size_t i = 0; i < nom_colonne.size(); ++i) {
            map[nom_colonne[i]] = i;
        }
    }

    void Projection(const std::vector<Database::Querying::Predicat_list> preds, const std::vector<std::string> nom_colonnes);
    void Selection(std::vector<std::string> nom_colonnes);

    int size()
    {
        return data.size();
    }
    int get_column_pos(std::string column_name)
    {
        return map[column_name];
    }

    Database::Querying::ColumnData get_value(std::string column_name, int pos_ind)
    {
        int pos = map[column_name];
        return data[pos]->getValue(pos_ind);
    }
    std::vector<Colonne*> get_data_ptr()
    {
        return data;
    }

    bool colonne_exist(std::string clef_testé)
    {
        return !(map.end() == map.find(clef_testé)); //this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
    }
};
