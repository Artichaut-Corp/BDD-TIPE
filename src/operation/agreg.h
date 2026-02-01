#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "data_process_system/meta-table.h"
#include "data_process_system/namingsystem.h"
#include "parser/expression.h"
#include "storage/types.h"

#ifndef agreg_H

#define agreg_H
namespace Database::QueryPlanning {

class ReturnType {
private:
    const ColonneNamesSet& m_Colonne;

    Parsing::AggrFuncType m_Operation;

public:
    ReturnType(const ColonneNamesSet& m_Colonne_, Parsing::AggrFuncType type_)
        : m_Colonne(m_Colonne_)
        , m_Operation(type_)
    {
    }

    explicit ReturnType(const ReturnType& other)
        : m_Operation(other.m_Operation)
        , m_Colonne(other.m_Colonne)
    {
    }

    Parsing::AggrFuncType GetType() { return m_Operation; };

    const ColonneNamesSet& GetColonne() const { return m_Colonne; };

    Database::ColumnData AppliqueOperation(std::unique_ptr<std::set<Database::ColumnData>> Values); // in case of Groupby
    //
    Database::ColumnData AppliqueOperationOnCol(const ColonneNamesSet& ColName, MetaTable* table);
};

class Final {
private:
    std::vector<ReturnType>* m_ColonneInfo;

    std::optional<std::vector<std::reference_wrapper<const ColonneNamesSet>>> m_ColumnsToGroupBy;

    // liste des m_Colonne par lesquelles trie, le booleen est vrai si on doit trier dans l'ordre decroissant
    std::optional<std::vector<std::pair<std::reference_wrapper<const ColonneNamesSet>, bool>>> m_OrderByCol;

    std::optional<std::pair<int, int>> m_Limite;

public:
    explicit Final(std::vector<ReturnType>* colonneInfo)
        : m_ColonneInfo(std::move(colonneInfo))
    {
    }

    Final(std::vector<ReturnType>* colonneInfo,
        const std::vector<ColonneNamesSet>& groupBy)
        : m_ColonneInfo(std::move(colonneInfo))
        , m_ColumnsToGroupBy(make_ref_vector(groupBy))
    {
    }

    Final(std::vector<ReturnType>* colonneInfo,
        const std::vector<ColonneNamesSet>& groupBy,
        const std::vector<std::pair<ColonneNamesSet, bool>>& orderBy)
        : m_ColonneInfo(std::move(colonneInfo))
        , m_ColumnsToGroupBy(make_ref_vector(groupBy))
    {
        std::vector<std::pair<std::reference_wrapper<const ColonneNamesSet>, bool>> tmp;
        tmp.reserve(orderBy.size());

        for (const auto& [col, desc] : orderBy)
            tmp.emplace_back(std::cref(col), desc);

        m_OrderByCol = std::move(tmp);
    }

    void AjouteGroupBy(const std::vector<std::reference_wrapper<const ColonneNamesSet>>& groupBy)
    {
        m_ColumnsToGroupBy = std::move(groupBy);
    }

    void AjouteOrderBy(
        const std::vector<std::pair<std::reference_wrapper<const ColonneNamesSet>, bool>>& orderBy)
    {
        m_OrderByCol = std::move(orderBy);
    }

    void AjouterLimite(int offset, int count)
    {
        m_Limite.emplace(offset, count);
    }

    std::size_t GetTailleClef() const
    {
        return m_ColumnsToGroupBy
            ? m_ColumnsToGroupBy->size()
            : 0;
    }

    std::chrono::high_resolution_clock::time_point AppliqueAgregateAndPrint(MetaTable* table, int benchmarking_INFO);

    void TrierListe(std::unordered_map<std::string, std::unique_ptr<std::vector<ColumnData>>>* ColumnNameToValues, std::vector<int>* IndicesVierge);

    bool CompareDeuxIndices(std::unordered_map<std::string, std::unique_ptr<std::vector<ColumnData>>>* ColumnNameToValues, int ind1, int ind2);
};

} // Database::QueryPlanning

#endif // !agreg_H
