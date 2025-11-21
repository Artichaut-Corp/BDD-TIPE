#include "meta-table.h"
#include "../parser/expression.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Database::QueryPlanning {

void MetaTable::Selection(const Parsing::BinaryExpression::Condition pred, const std::shared_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> nom_colonnes) // colonnes sur lesquelles on applique le filtre
{
    // Pour faire une selection, d'abord, il faut garder que les indices qui vérifient toute les conditions
    int taille = Columnsize();
    std::vector<int> indices_valides;
    std::unordered_map<std::string, ColumnData*> CoupleTesté;
    for (auto e : *nom_colonnes) {
        auto temp = new ColumnData {};
        for (auto f : e->GetAllFullNames()) {
            CoupleTesté[f] = temp;
        }
    }

    for (int i = 0; i < taille; i++) {
        for (auto e : *nom_colonnes) {
            const std::string& nom = e->GetMainName();
            *(CoupleTesté[nom]) = this->get_value(e,i);
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
    // quand on arrive ici, les élément dans indices_valides sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifier ne garder que les bons
    for (auto e : m_Tables) {
        e->AppliqueFiltre(&indices_valides);
    }
};

void MetaTable::Projection(std::unique_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> ColumnToSave)
{

    std::vector<std::shared_ptr<ColonneNamesSet>> difference;

    for (auto t : m_Tables) {
        for (auto r : *t->GetColumns()) {
            bool to_delete = true;
            for (auto s : *ColumnToSave) {
                if (*s == *r->get_name()) {
                    to_delete = false;
                    break;
                }
            }
            if (to_delete) {
                difference.push_back(r->get_name());
            }
        }
    }
    for(auto e : difference){
        m_MapColNameToTable.at(e->GetMainName())->DeleteCol(e);
    }
    UpdateMetaTable();
}
void MetaTable::Sort(std::shared_ptr<ColonneNamesSet> ColonneToSortBy)
{
    auto table = m_MapColNameToTable[ColonneToSortBy->GetMainName()];
    auto res = table->Sort(ColonneToSortBy);
    for(auto t : m_Tables){
        t->AppliqueFiltre(res);
    }
}

}