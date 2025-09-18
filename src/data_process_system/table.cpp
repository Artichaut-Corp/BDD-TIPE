#include "table.h"
#include "colonne.h"

#include <algorithm>
#include <cstddef>
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
void Table::Projection(
    const std::unique_ptr<std::vector<std::unique_ptr<Predicat_list>>> preds,
    const std::unique_ptr<std::vector<std::string>> nom_colonnes) // colonnes qui vont être modifié
// les éléments de la colonne en position i doivent vérifier le prédicat en positions i
{
    // Pour faire une projection, d'abord, il faut garder que les indices qui vérifient toute les conditions, ensuite, il faut modifier chaque colonne en ne gardant que ces indices, on rapelle que toute les colonne ont le même nombre d'indices mais ceux-ci diffère en valeur (voire explication.txt)
    size_t taille = (*data)[0]->size();
    std::vector<size_t> indices_valides;
    for (int i = 0; i < taille; i++) {
        indices_valides.push_back(i);
    }
    for (int i = 0; nom_colonnes->size(); i++) {
        std::string nom_colonne_testé = (*nom_colonnes)[i];
        size_t colonne_pos = map[nom_colonne_testé];
        std::unique_ptr<Colonne> colonne_testé = (*data)[colonne_pos];
        std::unique_ptr<Predicat_list> PredicatAVérifié = (*preds)[i];
        indices_valides.erase(std::remove_if(indices_valides.begin(), indices_valides.end(), [indices_valides, colonne_testé, PredicatAVérifié](size_t j) { return !PredicatAVérifié->Eval(colonne_testé->getValue(indices_valides[j])); }));
        // supprime tout les élément ne vérifiant pas le PredicatAVérifié
    }
    // quand on arrive ici, les élément dans indices_valides sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifié ne gardé que les bons
    std::unique_ptr<std::vector<size_t>> indices_valides_ptr = std::make_shared<std::vector<size_t>>(indices_valides);
    for (int i = 0; nom_colonnes->size(); i++) {
        std::string nom_colonne_testé = (*nom_colonnes)[i];
        size_t colonne_pos = map[nom_colonne_testé];
        std::unique_ptr<Colonne> colonne_testé = (*data)[colonne_pos];
        colonne_testé->garder_indice_valide(indices_valides_ptr);
    }
};

void Table::Selection(std::unique_ptr<std::vector<std::unique_ptr<std::string>>> nom_colonnes)
{
    for (size_t i = 0; i < nom_colonnes->size(); ++i) {
        size_t pos = map[*(*nom_colonnes)[i]];
        data->erase(data->begin() + pos-i);//prendre en compte le nombre d'indices supprimé
    }
}

} 
