#include "agreg.h"
#include "../utils/hashmap.h"
#include "../utils/table_utils.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// AVG_F,
// COUNT_F,
// MAX_F,
// MIN_F,
// SUM_F,
namespace Database::QueryPlanning {
Database::ColumnData ReturnType::AppliqueOperation(robin_hood::unordered_set<Database::ColumnData>& Values)
{
    if (Values.empty())
        return (uint16_t)0; // safeguard

    if (std::holds_alternative<Database::DbString>(*Values.begin())) {
        if (opération == Parsing::AggrFuncType::COUNT_F) {
            return static_cast<Database::DbInt>(Values.size());
        } else {
            throw Errors::Error(
                Errors::ErrorType::RuntimeError,
                "Agregate operation different from COUNT() is forbidden on string type",
                0, 0,
                Errors::ERROR_FORBIDEN_AGREGATE_ON_STRING);
        }
    }

    // Pour les types numériques
    if (opération == Parsing::AggrFuncType::AVG_F || opération == Parsing::AggrFuncType::SUM_F) {
        uint64_t sum = 0;
        size_t count = 0;
        for (const auto& e : Values) {
            if (std::holds_alternative<Database::DbInt>(e)) {
                sum += std::get<Database::DbInt>(e);
                ++count;
            } else if (std::holds_alternative<Database::DbInt64>(e)) {
                sum += std::get<Database::DbInt64>(e);
                ++count;
            }
        }
        if (opération == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / count);
        return static_cast<Database::DbInt>(sum);
    }

    if (opération == Parsing::AggrFuncType::MIN_F || opération == Parsing::AggrFuncType::MAX_F) {
        auto it = Values.begin();
        Database::ColumnData extremum = *it;
        ++it;
        for (; it != Values.end(); ++it) {
            if (opération == Parsing::AggrFuncType::MIN_F && *it < extremum)
                extremum = *it;
            if (opération == Parsing::AggrFuncType::MAX_F && *it > extremum)
                extremum = *it;
        }
        return extremum;
    }

    if (opération == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(Values.size());

    throw std::runtime_error(
        "Une Agrégation a été tentée alors qu'aucune fonction d'agrégation n'a été définie pour cette colonne");
}

Database::ColumnData ReturnType::AppliqueOperationOnCol(const std::string& ColName, Table* table)
{
    if (!table) 
        throw std::runtime_error("Table invalide");

    int n = table->Columnsize();
    if (n == 0)
        return static_cast<Database::DbInt>(0); // safeguard pour table vide

    //Sur les string, on ne peut count car max et min ne sont pas défini tout comme AVG et Sum
    ColumnData firstValue = table->get_value(ColName, 0);
    if (std::holds_alternative<Database::DbString>(firstValue)) {
        if (opération == Parsing::AggrFuncType::COUNT_F) {
            return static_cast<Database::DbInt>(n);
        } else {
            throw Errors::Error(
                Errors::ErrorType::RuntimeError,
                "Agregate operation different from COUNT() is forbidden on string type",
                0, 0,
                Errors::ERROR_FORBIDEN_AGREGATE_ON_STRING);
        }
    }

    // Pour les types numériques
    if (opération == Parsing::AggrFuncType::SUM_F || opération == Parsing::AggrFuncType::AVG_F) {
        uint64_t sum = 0;
        for (int i = 0; i < n; ++i) {
            ColumnData val = table->get_value(ColName, i);
            if (std::holds_alternative<Database::DbInt>(val)) {
                sum += std::get<Database::DbInt>(val);
            } else if (std::holds_alternative<Database::DbInt64>(val)) {
                sum += std::get<Database::DbInt64>(val);
            }
        }
        if (opération == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / n);
        return static_cast<Database::DbInt>(sum);
    }

    if (opération == Parsing::AggrFuncType::MIN_F || opération == Parsing::AggrFuncType::MAX_F) {
        ColumnData extremum = table->get_value(ColName, 0);
        for (int i = 1; i < n; ++i) {
            ColumnData val = table->get_value(ColName, i);
            if (opération == Parsing::AggrFuncType::MIN_F && val < extremum) extremum = val;
            if (opération == Parsing::AggrFuncType::MAX_F && val > extremum) extremum = val;
        }
        return extremum;
    }

    if (opération == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(n);

    throw std::runtime_error(
        "Une Agrégation a été tentée alors qu'aucune fonction d'agrégation n'a été définie pour cette colonne");
}



void Final::AppliqueAgregateAndPrint(Table* table)
{
    // Il faut savoir ce qui est le plus opti entre avoir une map pour chaque colonne agrégé ou avoir en valeur de chaque combi de clef un std::vector<std::unordered_set<ColumnData>>
    if (ColumnsToGroubBy.has_value()) {

        std::vector<std::string> AgregNames;
        std::unordered_map<std::string, Utils::Hash::MultiValueMapDyn> AgregMap; // la map en position i est celle de la colonne Agreg[column]
        for (auto x : (*table->GetColumnNames())) {
            bool est_présent = false;
            for (auto e : ColumnsToGroubBy.value()) {
                if (e == x) { // la colonne est dans les groupby
                    est_présent = true;
                }
            }
            if (!est_présent) {
                AgregNames.push_back(x);
                AgregMap[x] = Utils::Hash::MultiValueMapDyn {};
            }
        }

        // pour chaque ligne
        for (int i = 0; i < table->Columnsize(); i++) {
            // on créer la combinaison
            Utils::Hash::MultiKeyDyn KeyVec;// défini la comparaison entre clef et permet d'accéder à l'affichage des clefs
            for (auto e : ColumnsToGroubBy.value()) {
                auto temp = table->get_value(e, i);
                KeyVec.keys.push_back(temp);
            } // pour chaque colonne on l'ajoute dans la liste associé
            for (auto e : AgregNames) {
                auto temp = table->get_value(e, i);
                Utils::Hash::addValue(AgregMap[e], KeyVec, temp);
            }
        }
        for (auto& Return : ColonneInfo) {
            auto colName = Return.GetColonne();
            auto& map = AgregMap[colName];
            for (auto it = map.begin(); it != map.end(); ++it) {
                auto& values = it->second;
                ColumnData temp = Return.AppliqueOperation(values);
                values.clear();
                values.insert(temp);
            }
        }
        Database::Utils::AfficheAgregWithGroupByResult(ColumnsToGroubBy.value(),AgregNames,AgregMap);
    }else{
        std::vector<ColumnData> result;
        std::vector<std::string> ColName;
        for(auto e : ColonneInfo){
            ColName.push_back(e.GetColonne());
            result.push_back(e.AppliqueOperationOnCol(e.GetColonne(),table));
        }
        Database::Utils::AfficheAgregNoGroupByResult(ColName,result);
    }
}

}