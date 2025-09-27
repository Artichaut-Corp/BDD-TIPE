#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../algebrizer_types.h"
#include "colonne.h"

#ifndef TABLE_OP

#define TABLE_OP

namespace Database::QueryPlanning {

class Table {
private:
    std::shared_ptr<std::vector<std::shared_ptr<Colonne>>> data; // données immuables
    std::map<std::string, size_t> map; // permet de trouver la position d'une colonne à partir de son nom
    std::vector<std::string> Colonnes_names; // contient les noms de toute les colonnes présente dans la table (ces noms sont de la forme Table@colonnes) avec table étant la table originel( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci
    std::string Name;

public:
    Table(std::shared_ptr<std::vector<std::shared_ptr<Colonne>>> data_, const std::vector<std::string>& nom_colonne, std::string Name_) // noms et colonnes dans le bon ordre
        : data(std::move(data_))
        ,Name(Name_)
        ,Colonnes_names(nom_colonne)
    {
        for (size_t i = 0; i < nom_colonne.size(); ++i) {
            map[nom_colonne[i]] = i;
        }
    }

    void Selection(const std::shared_ptr<std::vector<std::shared_ptr<Predicat_list>>> preds, const std::shared_ptr<std::vector<std::string>> nom_colonnes);
    void Projection(std::unique_ptr<std::vector<std::string>> ColumnToSave);

    int size()
    {
        return data->size();
    }
    int get_column_pos(std::string column_name)
    {
        return map[column_name];
    }

    ColumnData get_value(std::string column_name, int pos_ind)
    {
        int pos = map[column_name];
        return (*data)[pos]->getValue(pos_ind);
    }
    std::shared_ptr<std::vector<std::shared_ptr<Colonne>>> get_data_ptr()
    {
        return data;
    }

    bool colonne_exist(std::string clef_testé)
    {
        return !(map.end() == map.find(clef_testé)); // this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
    }
    std::string Get_name(){return Name;};

    std::map<std::string, size_t> GetMap(){
        return map;
    }; 
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
