// Le but ici est de transformer l'arbre former par le parser un arbre très naïf qui seras ensuite modifié par l'optimiser
#include "../algebrizer/algebrizer.h"
#include "../data_process_system/table.h"
#include "../storage.h"
#include "../utils/BinaryTree.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Database::QueryPlanning {

std::string GetColumnFullName(const std::string& NomTablePrincipale, Database::Parsing::ColumnName* Colonne)
{
    if (Colonne->HaveTable()) {
        return Colonne->getColumnName(); // récupere le nom de cette colonne
    } else { // la colonne n'as pas de nom de table, on en conclu que c'est un colonne de la table principale, il faut donc rajouter le nom de cette table à son identifiant
        std::string nom_colonne = Colonne->getColumnName(); // récupere le nom de cette colonne
        return std::format("{}.{}", NomTablePrincipale, nom_colonne);
    }
}

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection,Storing::File * File,std::unordered_map<std::basic_string<char>, Database::Storing::TableInfo> * IndexGet)
{
    // Implémentation d'une conversion en arbre d'une query simple

    std::string Table_nom = Selection->getTable()->getTableName(); // ne peut pas être nullptr
    // récupérer la liste des colonne de retour,
    std::vector<ReturnType> colonnes_de_retour;
    std::vector<std::string> NomColonneDeRetour;
    std::vector<std::string> ColonneUsed;
    for (std::variant<Database::Parsing::SelectField, Database::Parsing::AggregateFunction> colonne_info : Selection->getFields()->getField()) { // permet de convertir m_fields list en un autre type plus utile
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Database::Parsing::SelectField>) { // si c'est un nom de colonne
                if (arg.isWildCard()) { // on vérifie si le nom de la colonne c'est pas "*"
                    
                } else { // il faut savoir de quelle table vient cette colonne
                    if (arg.m_Field.has_value()) { // on vérifie que y'as bien une valeur, c'est un type optional
                        std::string NomColonne = GetColumnFullName(Table_nom, &(arg.m_Field.value()));
                        colonnes_de_retour.push_back(ReturnType(NomColonne, AggrType::NOTHING_F));
                        ColonneUsed.push_back(NomColonne);
                        NomColonneDeRetour.push_back(NomColonne);

                    } else {
                        // bizare, c'est normalement impossible
                    }
                }
            } else if constexpr (std::is_same_v<T, Database::Parsing::AggregateFunction>) { // est une fonction d'agrégation
                if (!arg.isAll()) {
                    // on vérifie que y'as bien une valeur, c'est un type optinal
                    std::string nom_colonne = GetColumnFullName(Table_nom, arg.getColumnName());
                    ColonneUsed.push_back(nom_colonne);
                    NomColonneDeRetour.push_back(nom_colonne);

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

    //pour les join
    std::optional<std::vector<std::string>> Tables_secondaires;
    std::optional<std::vector<Join>> Join_list;
    if (Selection->getJoins() != nullptr) { // si il y as des join, on suppose que ce sont tous des join classique càd des inner join
        for (Database::Parsing::Join j : *Selection->getJoins()) {
            if (j.getJoinType() == Database::Parsing::JoinType::INNER_J) {
                Tables_secondaires->push_back(j.getTable()->getTableName());
                std::string colonne_gauche = GetColumnFullName(Table_nom, j.getLeftColumn());
                std::string colonne_droite = GetColumnFullName(Table_nom, j.getRightColumn());
                ColonneUsed.push_back(colonne_gauche);
                ColonneUsed.push_back(colonne_droite);

                Comparateur condition = Comparateur(Parsing::LogicalOperator::EQ); // dans tout les cas c'est un égal
                Join jointure = Join(condition, colonne_gauche, colonne_droite);
                Join_list->push_back(jointure);
            } else {
                // à implémenter
            }
        }
    }

    // il faut maintenant choper les conditions càd les where
    std::optional<Database::QueryPlanning::BinaryTree> clause; // Selection->getWhere();
    // a modifier une fois le BinaryTree type fini !

    if (Tables_secondaires.has_value()) { // cas avec des joins pas traité pour l'instant

    } else {        

        // pour l'instant le seul cas "fonctionnel" est le cas où la requete ressemble à : Select Personne.nom, Personne.age From Personne
        // on doit creer la table, pour cela on doit creer les racines et les Colonnes
        std::vector<std::shared_ptr<Racine>> Racines;
        Racines.reserve(ColonneUsed.size());
        std::vector<std::shared_ptr<Colonne>> Colonnes;
        Colonnes.reserve(ColonneUsed.size());
        for (std::string colonne_nom : ColonneUsed) {
            std::shared_ptr<Racine> RacinePtr = std::make_shared<Racine>(Racine(colonne_nom,File->Fd(),IndexGet));
            Racines.push_back(RacinePtr);
            std::shared_ptr<Colonne> ColonnePtr = std::make_shared<Colonne>(Colonne(RacinePtr, colonne_nom));
            Colonnes.push_back(ColonnePtr);
        }
        // étant donné qu'il n'y as qu'une seul Table on n'en créer qu'une seule avec tout les paramètre
        Table* table_principale = new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(Colonnes), ColonneUsed,Table_nom);
        
        // il faut maintenant creer la query
        Node Racine = Node(new Proj(NomColonneDeRetour)); //le tout dernier élément vérifie que les valeur restante sont celle de retour, donc on projete sur le type de retour
        
        Final filtre_fin = Final(colonnes_de_retour) ;

        std::vector<Table*> Tables;
        Tables.push_back(table_principale);
        Ikea Magasin = Ikea(Tables);

        Table* Table_Finale = Racine.Pronf(Magasin);
        filtre_fin.AfficheResultat(Table_Finale);
    }
}
};
