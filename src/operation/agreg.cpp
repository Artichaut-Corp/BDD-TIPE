#include "agreg.h"
#include "../utils/hashmap.h"
#include "../utils/printing_utils.h"

#include <gperftools/heap-profiler.h>
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
Database::ColumnData ReturnType::AppliqueOperation(std::set<Database::ColumnData>& Values)
{
    if (Values.empty())
        return (uint16_t)0; // safeguard

    if (std::holds_alternative<Database::DbString>(*Values.begin())) {
        if (m_Opération == Parsing::AggrFuncType::COUNT_F) {
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
    if (m_Opération == Parsing::AggrFuncType::AVG_F || m_Opération == Parsing::AggrFuncType::SUM_F) {
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
        if (m_Opération == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / count);
        return static_cast<Database::DbInt>(sum);
    }

    if (m_Opération == Parsing::AggrFuncType::MIN_F || m_Opération == Parsing::AggrFuncType::MAX_F) {
        auto it = Values.begin();
        Database::ColumnData extremum = *it;
        ++it;
        for (; it != Values.end(); ++it) {
            if (m_Opération == Parsing::AggrFuncType::MIN_F && *it < extremum)
                extremum = *it;
            if (m_Opération == Parsing::AggrFuncType::MAX_F && *it > extremum)
                extremum = *it;
        }
        return extremum;
    }

    if (m_Opération == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(Values.size());

    throw std::runtime_error(
        "Une Agrégation a été tentée alors qu'aucune fonction d'agrégation n'a été définie pour cette colonne");
}

Database::ColumnData ReturnType::AppliqueOperationOnCol(std::shared_ptr<ColonneNamesSet> ColName, std::shared_ptr<MetaTable> table)
{
    if (!table)
        throw std::runtime_error("Table invalide");

    int n = table->Columnsize();
    if (n == 0)
        return static_cast<Database::DbInt>(0); // safeguard pour table vide

    // Sur les string, on ne peut count car max et min ne sont pas défini tout comme AVG et Sum
    ColumnData firstValue = table->get_value(ColName, 0);
    if (std::holds_alternative<Database::DbString>(firstValue)) {
        if (m_Opération == Parsing::AggrFuncType::COUNT_F) {
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
    if (m_Opération == Parsing::AggrFuncType::SUM_F || m_Opération == Parsing::AggrFuncType::AVG_F) {
        uint64_t sum = 0;
        for (int i = 0; i < n; ++i) {
            ColumnData val = table->get_value(ColName, i);
            if (std::holds_alternative<Database::DbInt>(val)) {
                sum += std::get<Database::DbInt>(val);
            } else if (std::holds_alternative<Database::DbInt64>(val)) {
                sum += std::get<Database::DbInt64>(val);
            }
        }
        if (m_Opération == Parsing::AggrFuncType::AVG_F)
            return static_cast<Database::DbInt>(sum / n);
        return static_cast<Database::DbInt>(sum);
    }

    if (m_Opération == Parsing::AggrFuncType::MIN_F || m_Opération == Parsing::AggrFuncType::MAX_F) {
        ColumnData extremum = table->get_value(ColName, 0);
        for (int i = 1; i < n; ++i) {
            ColumnData val = table->get_value(ColName, i);
            if (m_Opération == Parsing::AggrFuncType::MIN_F && val < extremum)
                extremum = val;
            if (m_Opération == Parsing::AggrFuncType::MAX_F && val > extremum)
                extremum = val;
        }
        return extremum;
    }

    if (m_Opération == Parsing::AggrFuncType::COUNT_F)
        return static_cast<Database::DbInt>(n);

    throw std::runtime_error(
        "Une Agrégation a été tentée alors qu'aucune fonction d'agrégation n'a été définie pour cette colonne");
}

std::chrono::high_resolution_clock::time_point Final::AppliqueAgregateAndPrint(std::shared_ptr<MetaTable> table, int benchmarking_INFO)
{
    std::unordered_map<std::string, std::vector<ColumnData>*> ColumnNameToValues;
    std::vector<std::shared_ptr<ReturnType>>* PrintableColumnAndOrderByColumn = new std::vector<std::shared_ptr<ReturnType>>();
    for (auto Return : *m_ColonneInfo) {
        PrintableColumnAndOrderByColumn->push_back(Return);
    }
    if (m_OrderByCol.has_value()) {
        for (auto f : *m_OrderByCol) {
            bool est_deja_ajoute = false;
            for (auto e : *PrintableColumnAndOrderByColumn) { // faire gaffe au doublons
                if (*(f.first) == *(e->GetColonne())) {
                    est_deja_ajoute = true;
                }
            }
            if (!est_deja_ajoute) {
                PrintableColumnAndOrderByColumn->push_back(std::make_shared<ReturnType>( ReturnType(f.first, Parsing::AggrFuncType::NOTHING_F)));
            }
        }
    }
    if (m_ColumnsToGroupBy.has_value()) {

        // les colonne à garder sont celle qui ne sont pas group by et celle qui sont dans le group by mais dont ce sert après

        // on récupère les colonnes dont on doit garder les valeurs pour après donc celle qu'on affiche et celle qu'on order by

        std::vector<std::shared_ptr<ReturnType>>* UsefullColNotInGroup = new std::vector<std::shared_ptr<ReturnType>>();

        std::vector<std::shared_ptr<ReturnType>>* UsefullColInGroup = new std::vector<std::shared_ptr<ReturnType>>();

        std::unordered_map<std::shared_ptr<ReturnType>, int> ColInfoToKeyOrValueAndPos;

        int PosInValue = 0;
        for (auto Pc : *PrintableColumnAndOrderByColumn) {
            int PosInkey = 0;
            bool est_group_by = false;
            for (auto Gb : *m_ColumnsToGroupBy.value()) {
                if (*Gb == *Pc->GetColonne()) {
                    est_group_by = true;
                    UsefullColInGroup->push_back(Pc);
                    ColInfoToKeyOrValueAndPos[Pc] = PosInkey;
                }
                PosInkey++;
            }
            if (!est_group_by) {
                UsefullColNotInGroup->push_back(Pc);
                ColInfoToKeyOrValueAndPos[Pc] = PosInValue;
                PosInValue++;
            }
        }

        Utils::Hash::MultiValueMapDyn AgregMap;

        // pour chaque ligne
        for (int i = 0; i < table->Columnsize(); i++) {
            // on créer la combinaison
            Utils::Hash::MultiKeyDyn KeyVec; // défini la comparaison entre clef et permet d'accéder à l'affichage des clefs
            for (auto e : *m_ColumnsToGroupBy.value()) {
                auto temp = table->get_value(e, i);
                KeyVec.keys.push_back(temp);
            } // pour chaque colonne on l'ajoute dans la map associé
            for (auto e : *UsefullColNotInGroup) {
                auto temp = table->get_value(e->GetColonne(), i);
                Utils::Hash::addValue(AgregMap, KeyVec, temp, ColInfoToKeyOrValueAndPos[e]);
            }
        }

        for (auto ColName : *PrintableColumnAndOrderByColumn) { // creer l'endoit ou seront stocké les valeur utile après
            ColumnNameToValues[ColName->GetColonne()->GetMainName()] = new std::vector<ColumnData>;
        }

        for (auto it = AgregMap.begin(); it != AgregMap.end(); ++it) { // pour toute les combi de clef possible
            auto values = it->second; // récupère les valeur associé
            auto keys = it->first;
            for (auto Return : *UsefullColNotInGroup) { // on applique les agrégat
                auto ColName = Return->GetColonne();

                if (Return->GetType() != Parsing::AggrFuncType::NOTHING_F) { // dans colonneinfo il y a aussi les colonne qu'on retourne sans rien faire
                    ColumnData temp = Return->AppliqueOperation(values[ColInfoToKeyOrValueAndPos[Return]]); // performe l'm_Opération
                    ColumnNameToValues[ColName->GetMainName()]->push_back(temp);
                } else {
                    auto PosInValue = ColInfoToKeyOrValueAndPos[Return];
                    ColumnNameToValues[ColName->GetMainName()]->push_back(*values[PosInValue].begin());
                }
            }
            for (auto Return : *UsefullColInGroup) { // on applique les agrégat
                auto ColName = Return->GetColonne();
                auto PosInValue = ColInfoToKeyOrValueAndPos[Return];
                ColumnNameToValues[ColName->GetMainName()]->push_back(keys.GetValAt(PosInValue));
            }
        }

    } else {
        for (auto e : *PrintableColumnAndOrderByColumn) {
            std::vector<ColumnData>* ResultVector = new std::vector<ColumnData>;
            auto ColName = e->GetColonne();
            ColumnNameToValues[ColName->GetMainName()] = ResultVector;
            if (e->GetType() != Parsing::AggrFuncType::NOTHING_F) {
                ColumnNameToValues[ColName->GetMainName()]->push_back(e->AppliqueOperationOnCol(ColName, table));
            } else {
                for (int i = 0; i < table->Columnsize(); ++i) {
                    ColumnNameToValues[ColName->GetMainName()]->push_back(table->get_value(ColName, i));
                }
            }
        }
    }

    std::vector<int>* OrdreIndice = new std::vector<int>;
    OrdreIndice->reserve(ColumnNameToValues[ColumnNameToValues.begin()->first]->size());

    for (int i = 0; i < ColumnNameToValues[ColumnNameToValues.begin()->first]->size(); i++) {
        OrdreIndice->push_back(i);
    }
    if (m_OrderByCol.has_value() && (OrdreIndice->size() > 2)) {
        TrierListe(&ColumnNameToValues, OrdreIndice);
    }

    if (m_Limite.has_value()) {
        std::span<int> sub = std::span<int>(*OrdreIndice).subspan(m_Limite->first, m_Limite->second);

        auto fin = std::chrono::high_resolution_clock::now();
        if (benchmarking_INFO == 0) {
            Database::Utils::AfficheAgregSpan(&ColumnNameToValues, &sub, PrintableColumnAndOrderByColumn);
        }
        return fin;

    } else {
        auto fin = std::chrono::high_resolution_clock::now();
        if (benchmarking_INFO == 0) {
            Database::Utils::AfficheAgreg(&ColumnNameToValues, OrdreIndice, PrintableColumnAndOrderByColumn);
        }
        return fin;
    }
}

void Final::TrierListe(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, std::vector<int>* IndicesVierge)
{
    std::sort(IndicesVierge->begin(), IndicesVierge->end(), [&](int a, int b) {
        return CompareDeuxIndices(ColumnNameToValues, a, b);
        ;
    });
}

bool Final::CompareDeuxIndices(std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues, int ind1, int ind2)
{

    for (auto e : m_OrderByCol.value()) {
        auto ColonneCompared = e.first;
        auto estCroissant = e.second;
        auto are_equal = ((*((*ColumnNameToValues)[ColonneCompared->GetMainName()]))[ind1] == (*((*ColumnNameToValues)[ColonneCompared->GetMainName()]))[ind2]); // si les deux valeurs sont égale on passe à la condition suivante
        if (!are_equal) {
            return (estCroissant == (*((*ColumnNameToValues)[ColonneCompared->GetMainName()]))[ind1] < (*((*ColumnNameToValues)[ColonneCompared->GetMainName()]))[ind2]);
            /*
            estCroissant | ind1<ind2 | Result    (on entend par ind1<ind2, la comparaison dans la colonne des column data en position ind1 et ind2)
            ----------------------------------
                T        |     T      |    T
                T        |     F      |    F
                F        |     T      |    F
                F        |     F      |    T

            Cette table correspond à ce qui est renvoyé

            */
        }
    }
    return true; // valeur par défaut
}
}
