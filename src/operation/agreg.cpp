#include "agreg.h"

#include "utils/hashmap.h"
#include "utils/printing_utils.h"

#include <gperftools/heap-profiler.h>
#include <memory>
#include <set>
#include <span>
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
Database::ColumnData ReturnType::AppliqueOperation(std::unique_ptr<std::set<Database::ColumnData>> Values)
{
    if (Values->empty())
        return (uint16_t)0; // safeguard

    if (std::holds_alternative<Database::DbString>(*Values->begin())) {
        if (m_Operation == Parsing::AggrFuncType::COUNT_F) {
            return static_cast<Database::DbInt>(Values->size());
        } else {
            throw Errors::Error(
                Errors::ErrorType::RuntimeError,
                "Agregate operation different from COUNT() is forbidden on string type",
                0, 0,
                Errors::ERROR_FORBIDEN_AGREGATE_ON_STRING);
        }
    }

    // Pour les types numeriques
    if (m_Operation == Parsing::AggrFuncType::AVG_F || m_Operation == Parsing::AggrFuncType::SUM_F) {
        uint64_t sum = 0;
        size_t count = 0;
        for (const auto& e : *Values) {
            if (std::holds_alternative<Database::DbInt>(e)) {
                sum += std::get<Database::DbInt>(e);
                ++count;
            } else if (std::holds_alternative<Database::DbInt64>(e)) {
                sum += std::get<Database::DbInt64>(e);
                ++count;
            }
        }
        if (m_Operation == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / count);
        return static_cast<Database::DbInt>(sum);
    }

    if (m_Operation == Parsing::AggrFuncType::MIN_F || m_Operation == Parsing::AggrFuncType::MAX_F) {
        auto it = Values->begin();
        Database::ColumnData extremum = *it;
        ++it;
        for (; it != Values->end(); ++it) {
            if (m_Operation == Parsing::AggrFuncType::MIN_F && *it < extremum)
                extremum = *it;
            if (m_Operation == Parsing::AggrFuncType::MAX_F && *it > extremum)
                extremum = *it;
        }
        return extremum;
    }

    if (m_Operation == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(Values->size());

    throw std::runtime_error(
        "Une Agregation a ete tentee alors qu'aucune fonction d'agregation n'a ete definie pour cette colonne");
}

Database::ColumnData ReturnType::AppliqueOperationOnCol(const ColonneNamesSet& ColName, MetaTable* table)
{
    if (!table)
        throw std::runtime_error("Table invalide");

    int n = table->Columnsize();
    if (n == 0)
        return static_cast<Database::DbInt>(0); // safeguard pour table vide

    // Sur les string, on ne peut count car max et min ne sont pas defini tout comme AVG et Sum
    ColumnData firstValue = table->GetValue(ColName, 0);

    if (std::holds_alternative<Database::DbString>(firstValue)) {
        if (m_Operation == Parsing::AggrFuncType::COUNT_F) {
            return static_cast<Database::DbInt>(n);
        } else {
            throw Errors::Error(
                Errors::ErrorType::RuntimeError,
                "Agregate operation different from COUNT() is forbidden on string type",
                0, 0,
                Errors::ERROR_FORBIDEN_AGREGATE_ON_STRING);
        }
    }

    // Pour les types numeriques
    if (m_Operation == Parsing::AggrFuncType::SUM_F || m_Operation == Parsing::AggrFuncType::AVG_F) {
        uint64_t sum = 0;
        for (int i = 0; i < n; ++i) {
            ColumnData val = table->GetValue(ColName, i);
            if (std::holds_alternative<Database::DbInt>(val)) {
                sum += std::get<Database::DbInt>(val);
            } else if (std::holds_alternative<Database::DbInt64>(val)) {
                sum += std::get<Database::DbInt64>(val);
            }
        }
        if (m_Operation == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / n);
        return static_cast<Database::DbInt>(sum);
    }

    if (m_Operation == Parsing::AggrFuncType::MIN_F || m_Operation == Parsing::AggrFuncType::MAX_F) {
        ColumnData extremum = table->GetValue(ColName, 0);
        for (int i = 1; i < n; ++i) {
            ColumnData val = table->GetValue(ColName, i);
            if (m_Operation == Parsing::AggrFuncType::MIN_F && val < extremum)
                extremum = val;
            if (m_Operation == Parsing::AggrFuncType::MAX_F && val > extremum)
                extremum = val;
        }
        return extremum;
    }

    if (m_Operation == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(n);

    throw std::runtime_error(
        "Une Agregation a ete tentee alors qu'aucune fonction d'agregation n'a ete definie pour cette colonne");
}

std::chrono::high_resolution_clock::time_point Final::AppliqueAgregateAndPrint(MetaTable* table, int benchmarking_INFO)
{

    auto ColumnNameToValues = std::make_unique<std::unordered_map<std::string, std::unique_ptr<std::vector<ColumnData>>>>();

    auto PrintableColumnAndOrderByColumn = std::make_unique<std::vector<ReturnType>>();

    for (auto& Return : *m_ColonneInfo) {
        PrintableColumnAndOrderByColumn->push_back(Return);
    }

    if (m_OrderByCol.has_value()) {
        for (auto f : *m_OrderByCol) {

            bool est_deja_ajoute = false;

            // faire gaffe au doublons
            for (auto& e : *PrintableColumnAndOrderByColumn) {

                if (f.first == e.GetColonne()) {
                    est_deja_ajoute = true;
                }
            }

            if (!est_deja_ajoute) {

                PrintableColumnAndOrderByColumn->push_back(ReturnType(f.first, Parsing::AggrFuncType::NOTHING_F));
            }
        }
    }

    if (m_ColumnsToGroupBy.has_value()) {

        // les colonne à garder sont celle qui ne sont pas group by et celle qui sont dans le group by mais dont ce sert après

        // on recupère les colonnes dont on doit garder les valeurs pour après donc celle qu'on affiche et celle qu'on order by

        auto UsefullColNotInGroup = std::make_unique<std::vector<ReturnType>>();

        auto UsefullColInGroup = std::make_unique<std::vector<ReturnType>>();

        auto ColInfoToKeyOrValueAndPos = std::make_unique<std::unordered_map<ReturnType*, int>>();

        int PosInValue = 0;

        for (auto& Pc : *PrintableColumnAndOrderByColumn) {

            int PosInkey = 0;
            bool est_group_by = false;

            for (auto Gb : m_ColumnsToGroupBy.value()) {

                if (Gb == Pc.GetColonne()) {
                    est_group_by = true;
                    UsefullColInGroup->push_back(Pc);

                    ColInfoToKeyOrValueAndPos->at(&Pc) = PosInkey;
                }

                PosInkey++;
            }

            if (!est_group_by) {
                UsefullColNotInGroup->push_back(Pc);
                ColInfoToKeyOrValueAndPos->at(&Pc) = PosInValue;
                PosInValue++;
            }
        }

        Utils::Hash::MultiValueMapDyn AgregMap;

        // pour chaque ligne
        for (int i = 0; i < table->Columnsize(); i++) {

            // on creer la combinaison
            Utils::Hash::MultiKeyDyn KeyVec; // defini la comparaison entre clef et permet d'acceder à l'affichage des clefs
            //
            for (auto e : m_ColumnsToGroupBy.value()) {

                auto temp = table->GetValue(e, i);
                KeyVec.keys.push_back(temp);
            }

            // pour chaque colonne on l'ajoute dans la map associe
            for (auto& e : *UsefullColNotInGroup) {

                auto temp = table->GetValue(e.GetColonne(), i);

                Utils::Hash::addValue(AgregMap, KeyVec, temp, ColInfoToKeyOrValueAndPos->at(&e));
            }
        }

        // creer l'endoit ou seront stocke les valeur utile après
        for (auto& ColName : *PrintableColumnAndOrderByColumn) {
            ColumnNameToValues->at(ColName.GetColonne().GetMainName()) = std::make_unique<std::vector<ColumnData>>();
        }

        // pour toute les combi de clef possible
        for (auto it = AgregMap.begin(); it != AgregMap.end(); ++it) {

            // recupère les valeur associe
            std::vector<std::set<ColumnData>> values = it->second;
            auto keys = it->first;

            // on applique les agregat
            for (auto& Return : *UsefullColNotInGroup) {
                auto& ColName = Return.GetColonne();

                // dans colonneinfo il y a aussi les colonne qu'on retourne sans rien faire
                if (Return.GetType() != Parsing::AggrFuncType::NOTHING_F) {
                    // performe l'm_Operation

                    auto y = ColInfoToKeyOrValueAndPos->at(&Return);

                    auto r = std::make_unique<std::set<ColumnData>>(values.at(y));

                    ColumnData temp = Return.AppliqueOperation(std::move(r));

                    ColumnNameToValues->at(ColName.GetMainName())->push_back(temp);
                } else {
                    auto PosInValue = ColInfoToKeyOrValueAndPos->at(&Return);

                    ColumnNameToValues->at(ColName.GetMainName())->push_back(*values[PosInValue].begin());
                }
            }

            // on applique les agregat
            for (auto& Return : *UsefullColInGroup) {

                auto& ColName = Return.GetColonne();

                auto PosInValue = ColInfoToKeyOrValueAndPos->at(&Return);

                ColumnNameToValues->at(ColName.GetMainName())->push_back(keys.GetValAt(PosInValue));
            }
        }

    } else {
        for (auto& e : *PrintableColumnAndOrderByColumn) {

            auto ResultVector = std::make_unique<std::vector<ColumnData>>();

            auto& ColName = e.GetColonne();

            ColumnNameToValues->at(ColName.GetMainName()) = std::move(ResultVector);

            if (e.GetType() != Parsing::AggrFuncType::NOTHING_F) {

                auto t = e.AppliqueOperationOnCol(ColName, table);

                ColumnNameToValues->at(ColName.GetMainName())
                    ->push_back(t);
            } else {
                for (int i = 0; i < table->Columnsize(); ++i) {
                    ColumnNameToValues->at(ColName.GetMainName())->push_back(table->GetValue(ColName, i));
                }
            }
        }
    }

    auto OrdreIndice = std::make_unique<std::vector<int>>();

    OrdreIndice->reserve(ColumnNameToValues->at(ColumnNameToValues->begin()->first)->size());

    for (int i = 0; i < ColumnNameToValues->at(ColumnNameToValues->begin()->first)->size(); i++) {
        OrdreIndice->push_back(i);
    }

    if (m_OrderByCol.has_value() && (OrdreIndice->size() > 2)) {
        TrierListe(ColumnNameToValues.get(), OrdreIndice.get());
    }

    if (m_Limite.has_value()) {
        std::span<int> sub = std::span<int>(*OrdreIndice).subspan(m_Limite->first, m_Limite->second);

        auto fin = std::chrono::high_resolution_clock::now();
        if (benchmarking_INFO == 0) {
            Database::Utils::AfficheAgregSpan(std::move(ColumnNameToValues), &sub, std::move(PrintableColumnAndOrderByColumn));
        }
        return fin;

    } else {
        auto fin = std::chrono::high_resolution_clock::now();
        if (benchmarking_INFO == 0) {
            Database::Utils::AfficheAgreg(std::move(ColumnNameToValues), std::move(OrdreIndice), std::move(PrintableColumnAndOrderByColumn));
        }
        return fin;
    }
}

void Final::TrierListe(std::unordered_map<std::string, std::unique_ptr<std::vector<ColumnData>>>* ColumnNameToValues, std::vector<int>* IndicesVierge)
{
    std::sort(IndicesVierge->begin(), IndicesVierge->end(), [&](int a, int b) {
        return CompareDeuxIndices(ColumnNameToValues, a, b);
        ;
    });
}

bool Final::CompareDeuxIndices(std::unordered_map<std::string, std::unique_ptr<std::vector<ColumnData>>>* ColumnNameToValues, int ind1, int ind2)
{

    for (auto e : m_OrderByCol.value()) {
        const ColonneNamesSet& ColonneCompared = e.first;

        bool estCroissant = e.second;

        // si les deux valeurs sont egale on passe à la condition suivante

        ColumnData lhs = ColumnNameToValues->at(ColonneCompared.GetMainName())->at(ind1);
        ColumnData rhs = ColumnNameToValues->at(ColonneCompared.GetMainName())->at(ind2);

        if (!(lhs == rhs)) {
            return (estCroissant == (lhs < rhs));
            /*
            estCroissant | ind1<ind2 | Result    (on entend par ind1<ind2, la comparaison dans la colonne des column data en position ind1 et ind2)
            ----------------------------------
                T        |     T      |    T
                T        |     F      |    F
                F        |     T      |    F
                F        |     F      |    T

            Cette table correspond à ce qui est renvoye

            */
        }
    }
    return true; // valeur par defaut
}
}
