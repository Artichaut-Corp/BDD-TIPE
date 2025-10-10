
#include "../../lib/RobinHood/robin_hood.h"
#include <optional>
#include <string>

#include "../data_process_system/table.h"
#include "../parser/expression.h"
#include "../storage/types.h"
#include <vector>

#ifndef agreg_H

#define agreg_H
namespace Database::QueryPlanning {

static std::vector<int>& Sum();
static std::vector<int>& Average();
static std::vector<int>& Count();
static std::vector<int>& Min();
static std::vector<int>& Max();

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
    const Parsing::AggrFuncType GetType() { return opération; };

    std::string GetColonne() { return colonne; };

    Database::ColumnData AppliqueOperation(robin_hood::unordered_set<Database::ColumnData>& Values); // in case of Groupby
    Database::ColumnData AppliqueOperationOnCol(const std::string& ColName, Table* table);
};

class Final {
private:
    std::vector<ReturnType> ColonneInfo;
    std::optional<std::vector<std::string>> ColumnsToGroubBy;

public:
    Final(std::vector<ReturnType> ColonneInfo_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroubBy(std::nullopt)
    {
    }

    Final(std::vector<ReturnType> ColonneInfo_, std::vector<std::string> GroupByInfo_)
        : ColonneInfo(ColonneInfo_)
        , ColumnsToGroubBy(GroupByInfo_) {
        };

    void AjouteGroupBy(std::vector<std::string>& GroupBy){ColumnsToGroubBy=GroupBy;}
    int GetTailleClef() { return ColumnsToGroubBy->size(); }

    void AppliqueAgregateAndPrint(Table* table);
};

} // Database::QueryPlanning

#endif // !agreg_H
