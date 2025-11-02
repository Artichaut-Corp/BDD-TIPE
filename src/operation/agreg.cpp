#include "agreg.h"
#include "../utils/hashmap.h"
#include "../utils/printing_utils.h"
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

Database::ColumnData ReturnType::AppliqueOperationOnCol(ColonneNamesSet* ColName, Table* table)
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

void Final::AppliqueAgregateAndPrint(Table* table)
{
    std::unordered_map<std::string, std::vector<ColumnData>*> ColumnNameToValues;
    if (m_ColumnsToGroupBy.has_value()) {

        std::vector<ColonneNamesSet*>* AgregNames = new std::vector<ColonneNamesSet*>();
        int pos_in_map_incr = 0;
        std::unordered_map<ColonneNamesSet*, int> pos_in_map_map;
        Utils::Hash::MultiValueMapDyn AgregMap; // la map en position i est celle de la colonne Agreg[column]
        for (auto x : (*table->GetColumnNames())) {
            bool est_group_by = false;
            for (auto e : *m_ColumnsToGroupBy.value()) {
                if (e == x) { // la colonne est dans les groupby
                    est_group_by = true;
                }
            }
            if (!est_group_by) {
                AgregNames->push_back(x);
                pos_in_map_map[x] = pos_in_map_incr;
                pos_in_map_incr++;
            }
        }

        int posInKeyVec = 0;
        std::unordered_map<std::string, int> PosInKeyVecToPrint; // les élément dont la position est dans ce vecteur correspondent aux élement qui seront à afficher

        for (auto e : *m_ColumnsToGroupBy.value()) {
            bool est_retourné = false;
            for (auto f : *m_ColonneInfo) {
                if (*f->GetColonne() == *e) {
                    est_retourné = true;
                }
            }
            if (est_retourné) {
                PosInKeyVecToPrint[e->GetMainName()] = posInKeyVec;
            }
            posInKeyVec++;
        }

        // pour chaque ligne
        for (int i = 0; i < table->Columnsize(); i++) {
            // on créer la combinaison
            Utils::Hash::MultiKeyDyn KeyVec; // défini la comparaison entre clef et permet d'accéder à l'affichage des clefs
            for (auto e : *m_ColumnsToGroupBy.value()) {
                auto temp = table->get_value(e, i);
                KeyVec.keys.push_back(temp);
            } // pour chaque colonne on l'ajoute dans la map associé
            for (auto e : *AgregNames) {
                auto temp = table->get_value(e, i);
                Utils::Hash::addValue(AgregMap, KeyVec, temp, pos_in_map_map[e]);
            }
        }

        for (auto& Return : *m_ColonneInfo) {
            auto ColName = Return->GetColonne();
            ColumnNameToValues[ColName->GetMainName()] = new std::vector<ColumnData>;
        }

        for (auto it = AgregMap.begin(); it != AgregMap.end(); ++it) { // pour toute les combi de clef possible
            auto values = it->second; // récupère les valeur associé
            auto& keys = it->first;
            for (auto& Return : *m_ColonneInfo) { // on applique les agrégat
                auto ColName = Return->GetColonne();

                if (Return->GetType() != Parsing::AggrFuncType::NOTHING_F) { // dans colonneinfo il y a aussi les colonne qu'on retourne sans rien faire
                    ColumnData temp = Return->AppliqueOperation(values[pos_in_map_map[ColName]]); // performe l'm_Opération
                    ColumnNameToValues[ColName->GetMainName()]->push_back(temp);
                } else {
                    auto PosInKeys = PosInKeyVecToPrint[ColName->GetMainName()];
                    ColumnNameToValues[ColName->GetMainName()]->push_back(keys.keys[PosInKeys]);
                }
            }
        }

    } else {
        for (auto e : *m_ColonneInfo) {
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
        Database::Utils::AfficheAgregSpan(&ColumnNameToValues, &sub, m_ColonneInfo);
    } else {
        Database::Utils::AfficheAgreg(&ColumnNameToValues, OrdreIndice, m_ColonneInfo);
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
        auto ColonneCompared= e.first;
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