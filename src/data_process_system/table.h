#include <memory>
#include <unordered_map>
#include <vector>

#include "../algebrizer_types.h"
#include "colonne.h"

#ifndef TABLE_OP

#define TABLE_OP

namespace Database::QueryPlanning {

class Table {
private:
    std::unordered_map<std::string, std::shared_ptr<Colonne>> map; // permet de trouver la position d'une colonne à partir de son nom
    std::vector<ColonneNamesSet*> Colonnes_names; // contient les noms de toute les colonnes présente dans la table (ces noms sont de la forme Table@colonnes) avec table étant la table originel( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci
    TableNamesSet* Name;

public:
    Table(std::shared_ptr<std::vector<std::shared_ptr<Colonne>>> data_, TableNamesSet* Name_)
        : Name(Name_)
    {
        for (auto e : *data_) {
            for (auto n : e->get_name()->GetAllFullNames()) {
                map[n] = e;
            }
            map[e->get_name()->GetMainName()] = e;
            Colonnes_names.push_back(e->get_name());
        } 
    }

    void Selection(const Parsing::BinaryExpression::Condition pred, const std::unique_ptr<std::unordered_set<ColonneNamesSet*>> nom_colonnes);
    void Projection(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> ColumnToSave);

    int size()
    {
        return map.size();
    }
    int Columnsize()
    {
        return map[Colonnes_names[0]->GetMainName()]->size();
    }

    ColumnData get_value(ColonneNamesSet* column_name, int pos_ind)
    {
        auto temp = map[column_name->GetMainName()];
        return temp->getValue(pos_ind);
    }

    bool colonne_exist(ColonneNamesSet clef_testé)
    {
        return !(map.end() == map.find(clef_testé.GetMainName())); // this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
    }
    TableNamesSet* Get_name() { return Name; };

    std::unordered_map<std::string, std::shared_ptr<Colonne>> GetMap()
    {
        return map;
    };

    std::vector<std::shared_ptr<Colonne>>* get_data_ptr()
    {
        auto res = new std::vector<std::shared_ptr<Colonne>>();

        res->reserve(map.size());
        for (auto e : map) {
            res->emplace_back(e.second);
        }

        return res;
    }
    std::vector<ColonneNamesSet*>* GetColumnNames() { return &Colonnes_names; }
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
