#include "table.h"
#include "../parser/expression.h"
#include "colonne.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Database::QueryPlanning {

void Table::Selection(const Parsing::BinaryExpression::Condition pred, const std::unique_ptr<std::unordered_set<std::string>> nom_colonnes) // colonnes qui vont être modifié
// les éléments de la colonne en position i doivent vérifier le prédicat en positions i
{
    // Pour faire une projection, d'abord, il faut garder que les indices qui vérifient toute les conditions, ensuite, il faut modifier chaque colonne en ne gardant que ces indices, on rapelle que toute les colonne ont le même nombre d'indices mais ceux-ci diffère en valeur (voire explication.txt)
    int taille = Columnsize();
    std::vector<int> indices_valides;
    std::map<std::string, ColumnData> CoupleTesté;

    for (int i = 0; i < taille; i++) {
        for (auto e : *nom_colonnes) {
            CoupleTesté[e] = map[e]->getValue(i);
        }
        bool eval = false;
        if (std::holds_alternative<Parsing::Clause*>(pred)) {
            eval = std::get<Parsing::Clause*>(pred)->Eval(CoupleTesté);
        } else if (std::holds_alternative<Parsing::BinaryExpression*>(pred)) {
            eval = std::get<Parsing::BinaryExpression*>(pred)->Eval(CoupleTesté);
        } else {
            eval = true;
        }

        if (eval) {
            indices_valides.push_back(i); // ajoute  la ligne  vérifiant  le prédicat
        }
    }
    // quand on arrive ici, les élément dans indices_valides sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifié ne gardé que les bons
    std::shared_ptr<std::vector<int>> indices_valides_ptr = std::make_shared<std::vector<int>>(indices_valides);
    for (auto e : Colonnes_names) {
        std::shared_ptr<Colonne> colonne_testé = map[e];
        colonne_testé->garder_indice_valide(indices_valides_ptr);
    }
};

void Table::Projection(std::unique_ptr<std::vector<std::string>> ColumnToSave)
{

    std::vector<std::string> difference;
    std::sort(ColumnToSave->begin(), ColumnToSave->end());
    std::sort(Colonnes_names.begin(), Colonnes_names.end());

    std::set_difference(
        Colonnes_names.begin(), Colonnes_names.end(),
        ColumnToSave->begin(), ColumnToSave->end(),
        std::back_inserter(difference));

    for (int i = 0; i < difference.size(); ++i) {
        Colonnes_names.erase(std::find(Colonnes_names.begin(),Colonnes_names.end(),difference[i]));
        map.erase(difference[i]);
    }
}
}
