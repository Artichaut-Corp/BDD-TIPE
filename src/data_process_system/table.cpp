#include "table.h"
#include "../parser/expression.h"
#include "colonne.h"
#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Database::QueryPlanning {

std::vector<size_t> merge_sorted_unique(const std::vector<size_t>& a,
    const std::vector<size_t>& b)
{
    std::vector<size_t> result;
    result.reserve(a.size() + b.size());

    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] < b[j]) {
            result.push_back(a[i++]);
        } else if (b[j] < a[i]) {
            result.push_back(b[j++]);
        } else { // equal elements
            result.push_back(a[i]);
            ++i;
            ++j;
        }
    }

    // append remaining elements
    while (i < a.size())
        result.push_back(a[i++]);
    while (j < b.size())
        result.push_back(b[j++]);

    return result;
}
void Table::Selection(const Parsing::BinaryExpression::Condition pred, const std::unique_ptr<std::unordered_set<std::string>> nom_colonnes, std::string TablePrincipale) // colonnes qui vont être modifié
// les éléments de la colonne en position i doivent vérifier le prédicat en positions i
{
    // Pour faire une projection, d'abord, il faut garder que les indices qui vérifient toute les conditions, ensuite, il faut modifier chaque colonne en ne gardant que ces indices, on rapelle que toute les colonne ont le même nombre d'indices mais ceux-ci diffère en valeur (voire explication.txt)
    size_t taille = (*data)[0]->size();
    std::vector<size_t> indices_valides;
    std::map<std::string, ColumnData> CoupleTesté;

    for (int i = 0; i<taille; i++) {
        for (auto e : *nom_colonnes) {
                std::cout << e  << std::endl;

            CoupleTesté[e] = (*data)[map[e]]->getValue(i);
        }
        bool eval;
        if (std::holds_alternative<Parsing::Clause*>(pred)) {
            eval = std::get<Parsing::Clause*>(pred)->Eval(CoupleTesté, TablePrincipale);
        } else if (std::holds_alternative<Parsing::BinaryExpression*>(pred)) {
            eval = std::get<Parsing::BinaryExpression*>(pred)->Eval(CoupleTesté, TablePrincipale);
        } else {
            throw Errors::Error(Errors::ErrorType::RuntimeError, "Uknown type in the BinaryExpression Tree", 0, 0, Errors::ERROR_UNKNOW_TYPE_BINARYEXPR);
        }
        if(eval){
            indices_valides.push_back(i);// n'ajoute pas la ligne ne vérifiant pas le prédicat
        }
        
    }
    // quand on arrive ici, les élément dans indices_valides sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifié ne gardé que les bons
    std::unique_ptr<std::vector<size_t>> indices_valides_ptr = std::make_unique<std::vector<size_t>>(indices_valides);
    for (auto e : *nom_colonnes){
        std::shared_ptr<Colonne> colonne_testé = (*data)[map[e]];
        colonne_testé->garder_indice_valide(std::move(indices_valides_ptr));
    }
};

void Table::Projection(std::unique_ptr<std::vector<std::string>> ColumnToSave)
{

    std::vector<std::string> intersection;
    std::sort(ColumnToSave->begin(), ColumnToSave->end());
    std::sort(Colonnes_names.begin(), Colonnes_names.end());

    std::set_difference(
        Colonnes_names.begin(), Colonnes_names.end(),
        ColumnToSave->begin(), ColumnToSave->end(),
        std::back_inserter(intersection));

    for (size_t i = 0; i < intersection.size(); ++i) {
        size_t pos = map[intersection[i]];
        data->erase(data->begin() + pos - i); // prendre en compte le nombre d'indices supprimé
    }
}
}
