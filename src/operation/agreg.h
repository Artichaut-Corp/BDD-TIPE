
#include <optional>
#include <set>
#include <string>

#include "../data_process_system/table.h"
#include "../parser/expression.h"
#include "../storage/types.h"
#include <utility>
#include <vector>

#ifndef agreg_H

#define agreg_H
namespace Database::QueryPlanning {

class ReturnType {
private:
    std::string colonne;
    Parsing::AggrFuncType opération;

public:
    ReturnType(std::string colonne_, Parsing::AggrFuncType type_)
        : colonne(colonne_)
        , opération(type_)
    {
    }
    Parsing::AggrFuncType GetType() { return opération; };

    std::string GetColonne() { return colonne; };

    Database::ColumnData AppliqueOperation(const std::set<Database::ColumnData>& Values); // in case of Groupby
    Database::ColumnData AppliqueOperationOnCol(const std::string& ColName, Table* table);
};

class Final {
private:
    std::vector<ReturnType> ColonneInfo;
    std::optional<std::vector<std::string>> ColumnsToGroupBy;
    std::optional<std::vector<std::pair<std::string, bool>>> OrderByCol; // liste des colonne par lesquelles trié, le booléen est vrai si on doit triér dans l'ordre décroissant
    std::optional<std::pair<int, int>> Limite;

public:
    Final(std::vector<ReturnType> ColonneInfo_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroupBy(std::nullopt)
    {
    }

    Final(std::vector<ReturnType> ColonneInfo_, std::vector<std::string> GroupByInfo_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroupBy(GroupByInfo_) {
        };

    Final(std::vector<ReturnType> ColonneInfo_, std::vector<std::string> GroupByInfo_, std::vector<std::pair<std::string, bool>> OrderByCol_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroupBy(GroupByInfo_)
        , OrderByCol(OrderByCol_) {
        };
    Final(std::vector<ReturnType> ColonneInfo_, std::vector<std::string> GroupByInfo_, std::vector<std::pair<std::string, bool>> OrderByCol_, std::pair<int, int> limite_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroupBy(GroupByInfo_)
        , OrderByCol(OrderByCol_)
        , Limite(limite_) {
        };

    void AjouteGroupBy(std::vector<std::string>* GroupBy) { ColumnsToGroupBy = *GroupBy; }
    void AjouteOrderBy(std::vector<std::pair<std::string, bool>>* OrderBy) { OrderByCol = *OrderBy; }
    void AjouterLimite(int offset, int count) { Limite = std::pair<int, int>(offset, count); }

    int GetTailleClef() { return ColumnsToGroupBy->size(); }

    void AppliqueAgregateAndPrint(Table* table);
    void TrierListe(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, std::vector<int>* IndicesVierge);

    bool CompareDeuxIndices(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, int ind1, int ind2);
};

} // Database::QueryPlanning

#endif // !agreg_H
