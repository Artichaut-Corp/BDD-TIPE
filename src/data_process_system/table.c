#include "table.h"
#include "colonne.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Database::QueryPlanning {

std::vector<size_t> merge_sorted_unique(const std::vector<size_t> &a,
                                        const std::vector<size_t> &b) {
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
// je comprend pas l'erreur ???
void Table::Projection(
    const std::vector<Predicat_list> preds,
    const std::vector<std::string> nom_colonnes) // peut être opti
{
  std::vector<size_t> mauvais_indice; // dabord on recupère les indices faux
  for (size_t i = 0; i < nom_colonnes.size(); i++) {
    size_t pos = map[nom_colonnes[i]];
    Colonne *colonne_modifié = data[pos];
    std::vector<size_t> indice_faux = colonne_modifié->Projection(preds[i]);
    mauvais_indice = merge_sorted_unique(indice_faux, mauvais_indice);
  }
  for (Colonne *c : data) { // on les enlèves
    c->Enleve_indices(mauvais_indice);
  }
}

void Table::Selection(std::vector<std::string> nom_colonnes) {
  for (size_t i = 0; i < nom_colonnes.size(); ++i) {
    size_t pos = map[nom_colonnes[i]];
    data.erase(data.begin() + pos);
  }
}

} // namespace Database::QueryPlanning
