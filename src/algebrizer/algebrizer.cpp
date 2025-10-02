// Le but ici est de transformer l'arbre former par le parser un arbre très naïf qui seras ensuite modifié par l'optimiser
#include "../algebrizer/algebrizer.h"
#include "../data_process_system/table.h"
#include "../parser.h"
#include "../storage.h"
#include "../utils/table_utils.h"
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <variant>
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

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection, Storing::File* File, std::unordered_map<std::basic_string<char>, Database::Storing::TableInfo>* IndexGet)
{
    // Implémentation d'une conversion en arbre d'une query simple

    std::string TablePrincipaleNom = Selection->getTable()->getTableName(); // ne peut pas être nullptr
    // récupérer la liste des colonne de retour,
    std::vector<ReturnType> colonnes_de_retour;
    std::vector<std::string> NomColonneDeRetour; // à supprimer en fonction
    std::map<std::string, std::unordered_set<std::string>> TableNameToColumnList;

    for (std::variant<Parsing::SelectField, Parsing::AggregateFunction> colonne_info : Selection->getFields()->getField()) { // permet de convertir m_fields list en un autre type plus utile
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Parsing::SelectField>) { // si c'est un nom de colonne
                if (arg.isWildCard()) { // on vérifie si le nom de la colonne c'est pas "*"

                } else { // il faut savoir de quelle table vient cette colonne
                    if (arg.m_Field.has_value()) { // on vérifie que y'as bien une valeur, c'est un type optional
                        std::string NomColonne = GetColumnFullName(TablePrincipaleNom, &(arg.m_Field.value()));
                        colonnes_de_retour.push_back(ReturnType(NomColonne, Parsing::AggrFuncType::NOTHING_F));
                        TableNameToColumnList[NomColonne.substr(0, NomColonne.find("."))].emplace(NomColonne);
                        NomColonneDeRetour.push_back(NomColonne);

                    } else {
                        // bizare, c'est normalement impossible
                    }
                }
            } else if constexpr (std::is_same_v<T, Parsing::AggregateFunction>) { // est une fonction d'agrégation
                if (!arg.isAll()) {
                    // on vérifie que y'as bien une valeur, c'est un type optinal
                    std::string NomColonne = GetColumnFullName(TablePrincipaleNom, arg.getColumnName());
                    TableNameToColumnList[NomColonne.substr(0, NomColonne.find("."))].emplace(NomColonne);
                    NomColonneDeRetour.push_back(NomColonne);

                    colonnes_de_retour.push_back(ReturnType(NomColonne, arg.getType()));
                } else {
                    std::cout << "y'as une étoile\n"
                              << std::endl; // erreur
                }
            } else {
                std::cout << "type inconu parmis m_Fields lors de la création des colonnes de retour\n"
                          << std::endl; // erreur
            }
        },
            colonne_info);
    }

    // pour les join
    std::vector<std::string> tables_secondaires;
    std::vector<Join*> join_list;
    if (Selection->getJoins() != nullptr) { // si il y as des join, on suppose que ce sont tous des join classique càd des inner join
        for (Parsing::Join j : *Selection->getJoins()) {
            if (j.getJoinType() == Parsing::JoinType::INNER_J) {
                tables_secondaires.push_back(j.getTable()->getTableName());
                std::string colonne_gauche = GetColumnFullName(TablePrincipaleNom, j.getLeftColumn());
                std::string colonne_droite = GetColumnFullName(TablePrincipaleNom, j.getRightColumn());

                TableNameToColumnList[colonne_gauche.substr(0, colonne_gauche.find("."))].emplace(colonne_gauche);
                TableNameToColumnList[colonne_droite.substr(0, colonne_droite.find("."))].emplace(colonne_droite);

                Comparateur condition = Comparateur(Parsing::LogicalOperator::EQ); // dans tout les cas c'est un égal
                Join* jointure = new Join(condition, colonne_gauche, colonne_droite);
                join_list.push_back(jointure);
            } else {
                // à implémenter
                throw std::runtime_error("TODO: type de join pas traité");
            }
        }
    }

    Parsing::WhereClause* where = Selection->getWhere();
    Select* MainSelect;
    std::unordered_set<std::string>* ConditionColumn;
    Parsing::BinaryExpression::Condition Condition; //those variable are used two times,


    if (where != nullptr) { // il faut ajouter les colonnes utilisé dans la conditions avant de créer la table principale
        std::unordered_set<std::string>* ColonneTesté;

        Condition = where->m_Condition;

        if (std::holds_alternative<Parsing::BinaryExpression*>(Condition)) {
            std::get<Parsing::BinaryExpression*>(Condition)->FormatColumnName(TablePrincipaleNom);
            ConditionColumn = std::get<Parsing::BinaryExpression*>(Condition)->Column();
        } else {
            std::get<Parsing::Clause*>(Condition)->FormatColumnName(TablePrincipaleNom);
            ConditionColumn = std::get<Parsing::Clause*>(Condition)->Column();
        }
        for (auto e : *ConditionColumn) {
            TableNameToColumnList[e.substr(0, e.find("."))].insert(e);
        }
    }

    //  on doit creer la table principale, pour cela on doit creer les racines et les Colonnes
    std::vector<std::shared_ptr<Racine>> Racines;
    Racines.reserve(TableNameToColumnList[TablePrincipaleNom].size());
    std::vector<std::shared_ptr<Colonne>> Colonnes;
    Colonnes.reserve(TableNameToColumnList[TablePrincipaleNom].size());
    for (std::string colonne_nom : TableNameToColumnList[TablePrincipaleNom]) {
        std::shared_ptr<Racine> RacinePtr = std::make_shared<Racine>(Racine(colonne_nom, File->Fd(), IndexGet));
        Racines.push_back(RacinePtr);
        std::shared_ptr<Colonne> ColonnePtr = std::make_shared<Colonne>(Colonne(RacinePtr, colonne_nom));
        Colonnes.push_back(ColonnePtr);
    }
    // Maintenant que l'on as tout pour la table Principale on la créer
    Table* table_principale = new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(Colonnes), TablePrincipaleNom);
    Node RacineExec = Node(new Proj(NomColonneDeRetour, TablePrincipaleNom)); // le tout dernier élément vérifie que les valeur restante sont celle de retour, donc on projete sur le type de retour
    std::vector<Table> Tables;
    Tables.push_back(*table_principale); // on enregiste la table principale

    std::map<std::string, std::pair<Node*, bool>> TableToRootOfTableMap; // envoie l'endroit du plus petit noeud dans le plan d'éxécution où cette table est attendu (le booléen est là pour savoir si en cas de join, la table est le nom de droite ou de gauche)
    TableToRootOfTableMap[TablePrincipaleNom] = std::pair<Node*, bool>((&RacineExec), true);

    // il faut maintenant récupérer les conditions càd les where
    if (where != NULL) {// une foit la racine de l'arbre d'éxécution définie, on peut lui ajouter une selection si nécessaire
        MainSelect = new Select(std::make_unique<std::unordered_set<std::string>>(*ConditionColumn), Condition, TablePrincipaleNom);
        Node* Node_Select = new Node(MainSelect);
        RacineExec.AddChild(true, Node_Select);
        TableToRootOfTableMap[TablePrincipaleNom] = std::pair<Node*, bool>(Node_Select, true);
    }

    if (!tables_secondaires.empty()) { // si il y as des join
        //  on doit creer les autres tables
        //  pour cela on vas créer les racines et les colonne et donc les tables de chaque sous-table avant de créer l'arbre

        for (int i = 0; i < tables_secondaires.size(); i++) {
            std::vector<std::shared_ptr<Racine>> Racines;
            Racines.reserve(TableNameToColumnList[tables_secondaires[i]].size());
            std::vector<std::shared_ptr<Colonne>> Colonnes;
            Colonnes.reserve(TableNameToColumnList[tables_secondaires[i]].size());
            for (std::string colonne_nom : TableNameToColumnList[tables_secondaires[i]]) {
                std::shared_ptr<Racine> RacinePtr = std::make_shared<Racine>(Racine(colonne_nom, File->Fd(), IndexGet));
                Racines.push_back(RacinePtr);
                std::shared_ptr<Colonne> ColonnePtr = std::make_shared<Colonne>(Colonne(RacinePtr, colonne_nom));
                Colonnes.push_back(ColonnePtr);
            }
            // Maintenant que l'on as tout pour la table Principale on la créer
            Table* table_secondaire = new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(Colonnes), tables_secondaires[i]);
            Tables.push_back(*table_secondaire);

            std::string TableDéjàAjouter; // dans chaque création de jointure,il y a déjà une table présente dans l'arbre d'éxécution
            std::string TableGauche = join_list[i]->GetLTable();
            std::string TableDroite = join_list[i]->GetRTable();
            if (TableToRootOfTableMap.contains(TableGauche)) {
                TableDéjàAjouter = TableGauche;
            } else {
                TableDéjàAjouter = TableDroite;
            }
            Node* NoeudRacineTableDéjàAjouter = std::move(TableToRootOfTableMap[TableDéjàAjouter].first);
            bool EstGauche = TableToRootOfTableMap[TableDéjàAjouter].second;
            Node* EmplacementNouveauJoin = new Node(join_list[i]);
            NoeudRacineTableDéjàAjouter->AddChild(EstGauche, EmplacementNouveauJoin);

            TableToRootOfTableMap[TableDroite] = std::pair<Node*, bool>(EmplacementNouveauJoin, false);
            TableToRootOfTableMap[TableGauche] = std::pair<Node*, bool>(EmplacementNouveauJoin, true);
        }
    }

    Ikea* Magasin = new Ikea(Tables);
    RacineExec.printBT(std::cout);
    if (where != NULL) {
        std::cout << "\n et maintenant en descendant les sélections on a : \n  ";
        RacineExec.SelectionDescent(Magasin, MainSelect);
        RacineExec.printBT(std::cout);
    }
    Table* Table_Finale = RacineExec.Pronf(Magasin);
    Utils::AfficheResultat(Table_Finale);
}

};
