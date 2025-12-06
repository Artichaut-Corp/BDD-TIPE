
#include <memory>
#include <optional>
#include <set>

#include "../data_process_system/meta-table.h"
#include "../data_process_system/namingsystem.h"
#include "../parser/expression.h"
#include "../storage/types.h"
#include <utility>
#include <vector>

#ifndef agreg_H

#define agreg_H
namespace Database::QueryPlanning {

class ReturnType {
private:
    std::shared_ptr<ColonneNamesSet> m_Colonne;
    Parsing::AggrFuncType m_Opération;

public:
    ReturnType(std::shared_ptr<ColonneNamesSet> m_Colonne_, Parsing::AggrFuncType type_)
        : m_Colonne(m_Colonne_)
        , m_Opération(type_)
    {
    }
    Parsing::AggrFuncType GetType() { return m_Opération; };

    std::shared_ptr<ColonneNamesSet> GetColonne() const { return m_Colonne; };

    Database::ColumnData AppliqueOperation(std::set<Database::ColumnData>& Values); // in case of Groupby
    Database::ColumnData AppliqueOperationOnCol(std::shared_ptr<ColonneNamesSet> ColName, std::shared_ptr<MetaTable> table);
};

class Final {
private:
    std::vector<std::shared_ptr<ReturnType>>* m_ColonneInfo;
    std::optional<std::vector<std::shared_ptr<ColonneNamesSet>>*> m_ColumnsToGroupBy;
    std::optional<std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>>> m_OrderByCol; // liste des m_Colonne par lesquelles trié, le booléen est vrai si on doit triér dans l'ordre décroissant
    std::optional<std::pair<int, int>> m_Limite;

public:
    Final(std::vector<std::shared_ptr<ReturnType>>* m_ColonneInfo_)
        : m_ColonneInfo(m_ColonneInfo_)
        , m_ColumnsToGroupBy(std::nullopt)
    {
    }

    Final(std::vector<std::shared_ptr<ReturnType>>* m_ColonneInfo_, std::vector<std::shared_ptr<ColonneNamesSet>>* GroupByInfo_)
        : m_ColonneInfo(m_ColonneInfo_)
        , m_ColumnsToGroupBy(GroupByInfo_) {
        };

    Final(std::vector<std::shared_ptr<ReturnType>>* m_ColonneInfo_, std::vector<std::shared_ptr<ColonneNamesSet>>* GroupByInfo_, std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>> m_OrderByCol_)
        : m_ColonneInfo(m_ColonneInfo_)
        , m_ColumnsToGroupBy(GroupByInfo_)
        , m_OrderByCol(m_OrderByCol_) {
        };
    Final(std::vector<std::shared_ptr<ReturnType>>* m_ColonneInfo_, std::vector<std::shared_ptr<ColonneNamesSet>>* GroupByInfo_, std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>> m_OrderByCol_, std::pair<int, int> limite_)
        : m_ColonneInfo(m_ColonneInfo_)
        , m_ColumnsToGroupBy(GroupByInfo_)
        , m_OrderByCol(m_OrderByCol_)
        , m_Limite(limite_) {
        };

    void AjouteGroupBy(std::vector<std::shared_ptr<ColonneNamesSet>>* GroupBy) { m_ColumnsToGroupBy = GroupBy; }
    void AjouteOrderBy(std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>>* OrderBy) { m_OrderByCol = *OrderBy; }
    void AjouterLimite(int offset, int count) { m_Limite = std::pair<int, int>(offset, count); }

    int GetTailleClef() { return (*m_ColumnsToGroupBy)->size(); }

    std::chrono::high_resolution_clock::time_point  AppliqueAgregateAndPrint(std::shared_ptr<MetaTable> table,int benchmarking_INFO);
    void TrierListe(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, std::vector<int>* IndicesVierge);

    bool CompareDeuxIndices(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, int ind1, int ind2);
};

} // Database::QueryPlanning

#endif // !agreg_H
