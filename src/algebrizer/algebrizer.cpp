// Le but ici est de transformer l'arbre former par le parser un arbre très naïf qui seras ensuite modifié par l'optimiser
#include "../algebrizer/algebrizer.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "../storage.h"
namespace Database::QueryPlanning {

std::unique_ptr<Table> ConversionEnArbre_ET_excution(std::unique_ptr<Database::Parsing::SelectStmt> Selection, Storing:: )
{
    // Implémentation d'une conversion en arbre d'une query simple
    //  select sum(R.a) from R Join S ON R.c = S.b where 5<R.a<20 and 40<R.b<50 and 30<S.a<40

    // tout les commentaire vont décrypter comment ma conversion en Arbre marche pour cette query
    //--- Etape 1 : récupérer la Table utilisé, ici c'est 'pays' c'est facile elle est stocké dans Selection.m_Table
    std::string Table_nom = Selection->getTable()->getTableName();
    // récupérer la liste des colonne de retour,
    std::vector<ReturnType> colonnes_de_retour;
    for (std::variant<Database::Parsing::SelectField, Database::Parsing::AggregateFunction> colonne_info : Selection->getFields()->getField()) { // permet de convertir m_fields list en un autre type plus utile
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Database::Parsing::SelectField>) { // si c'est un nom de colonne
                if (arg.isWildCard()) { // on vérifie si le nom de la colonne c'est pas "*"
                    
                } else {
                    if (arg.m_Field.has_value()) { // on vérifie que y'as bien une valeur, c'est un type optional
                        if ((*arg.m_Field).HaveTable()) {
                            std::string nom_colonne = (*arg.m_Field).getColumnName(); // récupere le nom de cette colonne
                            colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::NOTHING_F));
                        } else { // la colonne n'as pas de nom de table, on en conclu que c'est un colonne de la table principale, il faut donc rajouter le nom de cette table à son identifiant
                            std::string nom_colonne = (*arg.m_Field).getColumnName(); // récupere le nom de cette colonne
                            nom_colonne = std::format("{}.{}", Table_nom, nom_colonne);
                            colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::NOTHING_F));
                        }
                    } else {
                        // bizare, est normalement impossible
                    }
                }
            } else if constexpr (std::is_same_v<T, Database::Parsing::AggregateFunction>) { // est une fonction d'agrégation
                if (!arg.isAll()) {

                    // on vérifie que y'as bien une valeur, c'est un type optinal
                    std::string nom_colonne;
                    if (arg.getColumnName()->HaveTable()) {
                        nom_colonne = arg.getColumnName()->getColumnName(); // récupere le nom de cette colonne
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::NOTHING_F));
                    } else { // la colonne n'as pas de nom de table, on en conclu que c'est un colonne de la table principale, il faut donc rajouter le nom de cette table à son identifiant
                        nom_colonne = arg.getColumnName()->getColumnName(); // récupere le nom de cette colonne
                        nom_colonne = std::format("{}.{}", Table_nom, nom_colonne);
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::NOTHING_F));
                    }
                    if (arg.getType() == Database::Parsing::AggrFuncType::AVG_F) {
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::AVG_F));
                    } else if (arg.getType() == Database::Parsing::AggrFuncType::COUNT_F) {
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::COUNT_F));
                    } else if (arg.getType() == Database::Parsing::AggrFuncType::MAX_F) {
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::MAX_F));
                    } else if (arg.getType() == Database::Parsing::AggrFuncType::MIN_F) {
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::MIN_F));
                    } else if (arg.getType() == Database::Parsing::AggrFuncType::SUM_F) {
                        colonnes_de_retour.push_back(ReturnType(nom_colonne, AggrType::SUM_F));
                    } else {
                        std::cout << "type inconu dans la conversion en arbre\n"; // erreur
                    }
                } else {
                    std::cout << "type inconu parmis AggrFuncType lors de la création des colonnes de retour\n"; // erreur
                }
            } else {
                std::cout << "type inconu parmis m_Fields lors de la création des colonnes de retour\n"; // erreur
            }
        },
            colonne_info);
    }
    std::optional<std::vector<std::string>> Table_secondaires;
    std::optional<std::vector<Join>> Join_list;
    if (Selection->getJoins() != nullptr) { // il y as des join, on suppose que ce sont tous des join classique des inner join
        for (Database::Parsing::Join j : *Selection->getJoins()) {
            if (j.getJoinType() == Database::Parsing::JoinType::INNER_J) {
                Table_secondaires->push_back(j.getTable()->getTableName());
                std::string colonne_gauche = j.getLeftColumn()->getColumnName();
                std::string colonne_droite = j.getRightColunm()->getColumnName();
                Comparateur condition = Comparateur(Parsing::LogicalOperator::EQ); // dans tout les cas c'est un égal
                Join jointure = Join(condition, colonne_gauche, colonne_droite);
                Join_list->push_back(jointure);
            } else {
                // à implémenter
            }
        }
    }
}
};
