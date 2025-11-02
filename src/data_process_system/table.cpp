#include "table.h"
#include "../parser/expression.h"
#include "colonne.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Database::QueryPlanning {

void Table::Selection(const Parsing::BinaryExpression::Condition pred, const std::unique_ptr<std::unordered_set<ColonneNamesSet*>> nom_colonnes) // colonnes qui vont être modifié
{
    // Pour faire une projection, d'abord, il faut garder que les indices qui vérifient toute les conditions, ensuite, il faut modifier chaque colonne en ne gardant que ces indices, on rapelle que toute les colonne ont le même nombre d'indices mais ceux-ci diffère en valeur (voire explication.txt)
    int taille = Columnsize();
    std::vector<int> indices_valides;
    std::unordered_map<std::string, ColumnData> valeurs;
    std::unordered_map<std::string, ColumnData*> CoupleTesté;
    for (auto e : *nom_colonnes) {
        valeurs[e->GetMainName()] = ColumnData {};
        for (auto f : e->GetAllFullNames()) {
            CoupleTesté[f] = &valeurs[e->GetMainName()];
        }
    }
    for (int i = 0; i < taille; i++) {
        for (auto e : *nom_colonnes) {
            const std::string& nom = e->GetMainName();
            auto itC = CoupleTesté.find(nom);
            auto itM = map.find(nom);
            if (itC != CoupleTesté.end() && itM != map.end() && itC->second != nullptr) {
                *(itC->second) = itM->second->getValue(i);
            } 
        }
        bool eval = false;
        if (std::holds_alternative<Parsing::Clause*>(pred)) {
            eval = std::get<Parsing::Clause*>(pred)->Eval(&CoupleTesté);
        } else if (std::holds_alternative<Parsing::BinaryExpression*>(pred)) {
            eval = std::get<Parsing::BinaryExpression*>(pred)->Eval(&CoupleTesté);
        } else {
            eval = true;
        }

        if (eval) {
            indices_valides.push_back(i); // ajoute  la ligne  vérifiant  le prédicat
        }
    }
    // quand on arrive ici, les élément dans indices_valides sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifié ne garder que les bons
    std::shared_ptr<std::vector<int>> indices_valides_ptr = std::make_shared<std::vector<int>>(indices_valides);
    for (auto e : Colonnes_names) {
        std::shared_ptr<Colonne> colonne_testé = map[e->GetMainName()];
        colonne_testé->garder_indice_valide(indices_valides_ptr);
    }
};

void Table::Projection(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> ColumnToSave)
{

    std::vector<ColonneNamesSet*> difference;

    for (auto Cn : Colonnes_names) {
        bool is_not_saved = true;
        for (auto Cts : *ColumnToSave) {
            if (*Cn == *Cts) {
                is_not_saved = false;
            }
        }
        if (is_not_saved) {
            difference.push_back(Cn);
        }
    }
    for (int i = 0; i < difference.size(); ++i) {
        Colonnes_names.erase(std::find(Colonnes_names.begin(), Colonnes_names.end(), difference[i]));
        for (auto n : difference[i]->GetAllFullNames()) {
            map.erase(n);
        }
    }
}
}
