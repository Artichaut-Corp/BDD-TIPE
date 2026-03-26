#include "meta-table.h"

#include "parser/expression.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Database::QueryPlanning {

void MetaTable::Selection(const Parsing::BinaryExpression::Condition& pred, const std::unique_ptr<std::unordered_set<ColonneNamesSet*>> nom_colonnes) // colonnes sur lesquelles on applique le filtre
{
    // Pour faire une selection, d'abord, il faut garder que les indices qui vérifient toute les conditions
    int size = Columnsize();

    std::vector<int> valid_indices;

    std::unordered_map<std::string, ColumnData*> tested_couples;

    for (auto& e : *nom_colonnes) {

        auto temp = new ColumnData {};

        for (auto f : e->GetAllFullNames()) {
            tested_couples[f] = temp;
        }
    }

    for (int i = 0; i < size; i++) {

        for (auto& e : *nom_colonnes) {
            const std::string& nom = e->GetMainName();
            *(tested_couples[nom]) = this->GetValue(*e, i);
        }

        bool eval = false;

        if (std::holds_alternative<Parsing::Clause>(pred)) {
            eval = std::get<Parsing::Clause>(pred).Eval(&tested_couples);
        } else if (std::holds_alternative<Parsing::BinaryExpression>(pred)) {
            eval = std::get<Parsing::BinaryExpression>(pred).Eval(&tested_couples);
        } else {
            eval = true;
        }

        if (eval) {
            valid_indices.push_back(i); // ajoute  la ligne  vérifiant  le prédicat
        }
    }

    // quand on arrive ici, les élément dans valid_indices sont les positions vérifiant tout les prédicat dans les liste des colonnes, il faut alors les modifier ne garder que les bons
    for (auto& e : m_Tables) {
        e->ApplyFilter(valid_indices);
    }
};

void MetaTable::Projection(std::unique_ptr<std::unordered_set<const ColonneNamesSet*>> ColumnToSave)
{

    std::vector<ColonneNamesSet*> difference;

    for (auto& t : m_Tables) {


        for (auto& r : *t->GetColumns()) {

            bool to_delete = true;

            for (auto& s : *ColumnToSave) {
                if (*s == r->GetName()) {
                    to_delete = false;
                    break;
                }
            }
            if (to_delete) {
                difference.push_back(&r->GetName());
            }
        }
    }

    for (auto e : difference) {
        GetTableByColName(*e).DeleteCol(*e);
    }

    UpdateMetaTable();
}

void MetaTable::Sort(const ColonneNamesSet& ColonneToSortBy)
{
    int table_ind = m_MapColNameToTable[ColonneToSortBy.GetMainName()];

    const Table& table = GetTableFromMap(table_ind);

    auto res = table.Sort(ColonneToSortBy);

    for (auto& t : m_Tables) {
        t->ApplyFilter(*res);
    }
}

}
