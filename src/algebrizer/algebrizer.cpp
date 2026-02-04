// Le but ici est de transformer l'arbre former par le parser un arbre très naïf qui seras ensuite modifié par l'optimiser
#include "algebrizer/algebrizer.h"
#include "data_process_system/meta-table.h"
#include "data_process_system/namingsystem.h"
#include "parser.h"
#include "storage.h"
#include "utils/printing_utils.h"
#include "utils/union_find.h"

#include <chrono>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace Database::QueryPlanning {

ColonneNamesSet* ConvertToStandardColumnName(TableNamesSet& NomTablePrincipale, Database::Parsing::ColumnName* colonne, std::unordered_map<std::string, std::unique_ptr<TableNamesSet>>& variation_of_tablename_to_main_table_name)
{
   ColonneNamesSet* standard_name = nullptr;

    if (colonne->HaveTable()) {

        TableNamesSet* table = variation_of_tablename_to_main_table_name.at(colonne->GetTable()).get();

        std::string full_name = colonne->getColumnName();

        // récupere le nom de cette colonne
        standard_name = new ColonneNamesSet(full_name, colonne->GetAlias(), std::unique_ptr<TableNamesSet>(table));
    } else {
        // la colonne n'as pas de nom de table, on en conclut que c'est une colonne de la table principale, il faut donc rajouter le nom de cette table à son identifiant

        auto parent_tbl = std::unique_ptr<TableNamesSet>(&NomTablePrincipale);

        standard_name = new ColonneNamesSet(colonne->getColumnName(),
            colonne->GetAlias(),
            std::move(parent_tbl));
    }

    return standard_name;
}

std::unique_ptr<TableNamesSet> ConvertToStandardTableName(Database::Parsing::TableName* Table, std::unordered_map<std::string, std::unique_ptr<TableNamesSet>>& variation_of_tablename_to_main_table_name)
{
    std::unique_ptr<TableNamesSet> standard_name = std::make_unique<TableNamesSet>(Table->getTableName());

    variation_of_tablename_to_main_table_name[standard_name->GetMainName()] = std::unique_ptr<TableNamesSet>(standard_name.get());

    for (auto e : *Table->GetAlias()) {
        standard_name->AddAlias(e);

        variation_of_tablename_to_main_table_name[e] = std::unique_ptr<TableNamesSet>(standard_name.get());
    }

    return standard_name;
}

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection, Storing::File* File, std::unordered_map<std::string, Database::Storing::TableInfo>* IndexGet, std::unique_ptr<std::vector<int>> param)
{
    auto deb = std::chrono::high_resolution_clock::now();

    int descend_select = param->at(0);
    int type_of_join = param->at(1);
    int InserProj = param->at(2);
    int optimize_BinaryExpr = param->at(3);
    int Ordering_Join = param->at(4);
    int benchmarking = param->at(5);

    // Implémentation d'une conversion en arbre d'une query simple
    auto variation_of_tablename_to_main_table_name = std::make_unique<std::unordered_map<std::string, std::unique_ptr<TableNamesSet>>>();

    // ne peut pas être nullptr
    std::unique_ptr<TableNamesSet> TablePrincipaleNom = ConvertToStandardTableName(Selection->getTable(), *variation_of_tablename_to_main_table_name);

    // récupérer la liste des colonne de retour,
    auto colonnes_de_retour = std::make_unique<std::vector<ReturnType>>();

    std::unordered_map<std::string, std::unordered_set<ColonneNamesSet*>*> TableNameToColumnList;

    // Could be rewritten to use unique_ptr
    auto UsefullColumnForAggrAndOutput = std::make_unique<std::unordered_set<const ColonneNamesSet*>>();

    bool IsAgregate = false;

    // pour les join
    std::vector<std::unique_ptr<TableNamesSet>> tables_secondaires;

    std::vector<Join*> join_list;

    std::vector<Parsing::Join>* joins = Selection->getJoins();

    if (joins != nullptr) { // si il y as des join, on suppose que ce sont tous des join classique càd des inner join
        for (const Parsing::Join& j : *joins) {

            if (j.getJoinType() == Parsing::JoinType::INNER_J) {

                tables_secondaires.push_back(ConvertToStandardTableName(j.getTable(), *variation_of_tablename_to_main_table_name));

                ColonneNamesSet* colonne_gauche = (ConvertToStandardColumnName(*TablePrincipaleNom, j.getLeftColumn(), *variation_of_tablename_to_main_table_name));

                ColonneNamesSet* colonne_droite = (ConvertToStandardColumnName(*TablePrincipaleNom, j.getRightColumn(), *variation_of_tablename_to_main_table_name));

                std::unordered_set<ColonneNamesSet*>* sd = TableNameToColumnList.at(colonne_droite->GetTableSet()->GetMainName());

                sd->insert(colonne_droite);

                std::unordered_set<ColonneNamesSet*>* sg = TableNameToColumnList.at(colonne_gauche->GetTableSet()->GetMainName());

                sg->insert(colonne_gauche);

                Comparateur condition = Comparateur(Parsing::LogicalOperator::EQ); // dans tout les cas c'est un égal

                Join* jointure = new Join(condition, *colonne_gauche, *colonne_droite);

                join_list.push_back(jointure);
            } else {
                // à implémenter
                throw std::runtime_error("TODO: type de join pas traité");
            }
        }
    }

    for (std::variant<Parsing::SelectField, Parsing::AggregateFunction> colonne_info : Selection->getFields()->getField()) { // permet de convertir m_fields list en un autre type plus utile
        if (std::holds_alternative<Parsing::SelectField>(colonne_info)) {
            auto arg = std::get<Parsing::SelectField>(colonne_info);
            if (arg.isWildCard()) { // on vérifie si le nom de la colonne c'est pas "*"

            } else { // il faut savoir de quelle table vient cette colonne
                if (arg.m_Field.has_value()) { // on vérifie que y'as bien une valeur, c'est un type optional
                    auto col = (ConvertToStandardColumnName(*TablePrincipaleNom, &(arg.m_Field.value()), *variation_of_tablename_to_main_table_name));

                    UsefullColumnForAggrAndOutput->insert(col);

                    std::unordered_set<ColonneNamesSet*>* s;

                 

                     if (TableNameToColumnList.contains(col->GetTableSet()->GetMainName()))
                     {
                         s = TableNameToColumnList.at(col->GetTableSet()->GetMainName());

                         s->insert(col);
                     } else {
                        s = new std::unordered_set<ColonneNamesSet*>;




                        s->insert(col);

                        TableNameToColumnList.insert({
                            col->GetTableSet()->GetMainName(), s});
                     }

                    colonnes_de_retour->emplace_back(std::move(*col), Parsing::AggrFuncType::NOTHING_F);

                } else {
                    // bizare, c'est normalement impossible
                }
            }
        } else if (std::holds_alternative<Parsing::AggregateFunction>(colonne_info)) { // est une fonction d'agrégation

            auto arg = std::get<Parsing::AggregateFunction>(colonne_info);

            if (!arg.isAll()) {
                // on vérifie que y'as bien une valeur, c'est un type optinal
                ColonneNamesSet* NomColonne = ConvertToStandardColumnName(*TablePrincipaleNom, arg.getColumnName(), *variation_of_tablename_to_main_table_name);

                std::unordered_set<ColonneNamesSet*>* s = TableNameToColumnList.at(NomColonne->GetTableSet()->GetMainName());

                s->insert(NomColonne);

                colonnes_de_retour->push_back(ReturnType(*NomColonne, arg.getType()));

                IsAgregate = true;

                UsefullColumnForAggrAndOutput->insert(NomColonne);

            } else {
                std::cout << "y'as une étoile\n"
                          << std::endl; // erreur
            }
        } else {
            std::cout << "type inconu parmis m_Fields lors de la création des colonnes de retour\n"
                      << std::endl; // erreur
        }
    }

    auto AppliqueAggr = Final(colonnes_de_retour.get());

    // permet de créer les agrégation si il y en as
    if (IsAgregate) {

        Parsing::GroupByClause* Groupby = Selection->getGroupBy();

        if (Groupby != nullptr) {

            std::vector<std::reference_wrapper<const ColonneNamesSet>> ColumnGroupByed = {};

            for (auto e : Groupby->getByItems()) {
                ColonneNamesSet* NomColonne = ConvertToStandardColumnName(*TablePrincipaleNom, e.getColName(), *variation_of_tablename_to_main_table_name);

                std::unordered_set<ColonneNamesSet*>* s = TableNameToColumnList.at(NomColonne->GetTableSet()->GetMainName());

                s->insert(NomColonne);

                ColumnGroupByed.push_back(*NomColonne);

                UsefullColumnForAggrAndOutput->insert(NomColonne);
            }

            AppliqueAggr
                .AjouteGroupBy(ColumnGroupByed);
        }
    }

    // permet de créer les OrderBy si il y en as
    Parsing::OrderByClause* order = Selection->getOrderBy();

    bool IsOrderBy = false;

    if (order != nullptr) {

        IsOrderBy = true;

        std::vector<std::pair<std::reference_wrapper<const ColonneNamesSet>, bool>> OrderVect = {};

        for (auto e : order->getByItems()) {
            ColonneNamesSet* NomColonne = ConvertToStandardColumnName(*TablePrincipaleNom, e.getColName(), *variation_of_tablename_to_main_table_name);

            bool est_présent = false;

            for (auto& x : *colonnes_de_retour) {
                if (x.GetColonne() == *NomColonne) {
                    est_présent = true;
                }
            }

            // évite les doublons dans la projection finale et dans la création des tables
            if (!est_présent) {

                std::unordered_set<ColonneNamesSet*>* s = TableNameToColumnList.at(NomColonne->GetTableSet()->GetMainName());

                s->insert(NomColonne);

                UsefullColumnForAggrAndOutput->insert(NomColonne);
            }

            // on inverse le Desc car il est vrai si c'est inversé et dans la suite on considère que si c'est vrai alors c'est Asc
            OrderVect.push_back({ *NomColonne, !e.isDsc() });
        }

        AppliqueAggr.AjouteOrderBy(OrderVect);
    }

    bool IsLimite = false;

    Parsing::Limit* Limite = Selection->getLimit();

    if (Limite != nullptr) {

        IsLimite = true;

        AppliqueAggr.AjouterLimite(Limite->getOffset(), Limite->getCount());
    }

    Parsing::WhereClause* where = Selection->getWhere();

    Select* MainSelect;

    std::unordered_set<ColonneNamesSet*>* ConditionColumn;

    Parsing::BinaryExpression::Condition cond; // those variable are used two times,

    // il faut ajouter les colonnes utilisé dans la conditions avant de créer la table principale
    if (where != nullptr) {

        ConditionColumn = where->GetConditionColumnNames(TablePrincipaleNom.get());

        for (auto& NomColonne : *ConditionColumn) {

            bool est_présent = false;

            for (auto& x : *colonnes_de_retour) {
                if (x.GetColonne() == *NomColonne) {
                    est_présent = true;
                }
            }
            // évite les doublons dans la projection finale et dans la création des tables
            if (!est_présent) {

                std::unordered_set<ColonneNamesSet*>* s = TableNameToColumnList.at(NomColonne->GetTableSet()->GetMainName());

                s->insert(NomColonne);
            }
        }
    }

    //  on doit creer la table principale, pour cela on doit creer les racines et les Colonnes
    std::vector<Racine> Racines;

    Racines.reserve(TableNameToColumnList[TablePrincipaleNom->GetMainName()]->size());

    std::unordered_set<std::unique_ptr<ColonneNamesSet>> ColonneAlreadyCreate;

    for (auto colonne_nom : *TableNameToColumnList.at(TablePrincipaleNom->GetMainName())) {

        bool est_déjà_ajouté = false;

        for (auto& e : ColonneAlreadyCreate) {

            if (*colonne_nom == *e) {

                est_déjà_ajouté = true;

                colonne_nom->FusionColumn(*e.get());

                break;
            }
        }

        if (!est_déjà_ajouté) {

            Racines.emplace_back(colonne_nom, File->Fd(), IndexGet);

            ColonneAlreadyCreate.emplace(colonne_nom);
        }
    }

    // Maintenant que l'on as tout pour la table Principale on la créer
    std::unique_ptr<MetaTable> table_principale = std::make_unique<MetaTable>(Racines, *TablePrincipaleNom.get());

    // le tout dernier élément vérifie que les valeur restante sont celle de retour, donc on projete sur le type de retour
    Node RacineExec = Node(new Proj(std::move(UsefullColumnForAggrAndOutput), *TablePrincipaleNom.get()));

    std::vector<std::unique_ptr<MetaTable>> Tables;

    // on enregiste la table principale
    Tables.push_back(std::move(table_principale));

    auto RacineMainTable = &RacineExec;

    std::unordered_map<std::string, std::pair<Node*, bool>> TableToRootOfTableMap; // envoie l'endroit du plus petit noeud dans le plan d'éxécution où cette table est attendu (le booléen est là pour savoir si en cas de join, la table est le nom de droite ou de gauche)
    TableToRootOfTableMap[TablePrincipaleNom->GetMainName()] = std::pair<Node*, bool>((&RacineExec), true);

    // il faut maintenant récupérer les conditions càd les where
    if (where != NULL) {
        // une foit la racine de l'arbre d'éxécution définie, on peut lui ajouter une selection si nécessaire
        MainSelect = new Select(std::unique_ptr<std::unordered_set<ColonneNamesSet*>>(ConditionColumn), cond, *TablePrincipaleNom.get());

        auto Node_Select = std::make_unique<Node>(MainSelect);

        RacineExec.AddChild(true, Node_Select.get());

        TableToRootOfTableMap[TablePrincipaleNom->GetMainName()] = std::pair<Node*, bool>(Node_Select.get(), true);

        RacineMainTable = Node_Select.get();
    }

    if (!tables_secondaires.empty()) { // si il y as des join
        //  on doit creer les autres tables
        //  pour cela on vas créer les racines et les colonne et donc les tables de chaque sous-table avant de créer l'arbre

        for (int i = 0; i < tables_secondaires.size(); i++) {

            std::vector<Racine> Racines;

            Racines.reserve(TableNameToColumnList[tables_secondaires[i]->GetMainName()]->size());

            ColonneAlreadyCreate.clear();

            for (
                auto& colonne_nom : *TableNameToColumnList[tables_secondaires[i]->GetMainName()]) {

                bool est_déjà_ajouté = false;

                for (auto& e : ColonneAlreadyCreate) {
                    if (*colonne_nom == *e) {
                        est_déjà_ajouté = true;

                        colonne_nom->FusionColumn(*e.get());
                        break;
                    }
                }

                if (!est_déjà_ajouté) {

                    Racines.emplace_back(colonne_nom, File->Fd(), IndexGet);

                    ColonneAlreadyCreate.emplace(colonne_nom);
                }
            }

            // Maintenant que l'on as tout pour la table Principale on la créer
            std::unique_ptr<MetaTable> table_secondaire = std::make_unique<MetaTable>(Table(Racines, *tables_secondaires[i].get()));

            Tables.push_back(std::move(table_secondaire));

            // dans chaque création de jointure,il y a déjà une table présente dans l'arbre d'éxécution
            const TableNamesSet* already_added_table = nullptr;

            const TableNamesSet& left_table = join_list[i]->GetLTable();

            const TableNamesSet& right_table = join_list[i]->GetRTable();

            if (TableToRootOfTableMap.contains(left_table.GetMainName())) {
                already_added_table = &left_table;
            } else {
                already_added_table = &right_table;
            }

            Node* NoeudRacineTableDéjàAjouter = std::move(TableToRootOfTableMap[already_added_table->GetMainName()].first);

            bool EstGauche = TableToRootOfTableMap[already_added_table->GetMainName()].second;

            Node* new_join_location = new Node(join_list[i]);

            NoeudRacineTableDéjàAjouter->AddChild(EstGauche, new_join_location);

            TableToRootOfTableMap[right_table.GetMainName()] = std::pair<Node*, bool>(new_join_location, false);

            TableToRootOfTableMap[left_table.GetMainName()] = std::pair<Node*, bool>(new_join_location, true);
        }
    }

    Ikea* Magasin = new Ikea(Tables);

    if (benchmarking == 0) {
        RacineExec.printBT(std::cout);
    }

    if (where != NULL and optimize_BinaryExpr == 1) {

        auto SelectNode = RacineExec.GetLeftPtr();

        if (SelectNode == nullptr) {
            std::cout << "Absurdité, where n'est pas null mais aucun select n'est présent\n"
                      << std::endl; // erreur
        } else {
            auto SelectAct = SelectNode->GetAction();

            if (std::holds_alternative<Select*>(SelectAct)) {
                Select* op = std::get<Select*>(SelectAct);

                // if the cond is a clause or a tautology, we can't otpimize it
                if (std::holds_alternative<Parsing::BinaryExpression>(op->GetCond().get())) {

                    Parsing::BinaryExpression& cond = std::get<Database::Parsing::BinaryExpression>(op->GetCond().get());

                    const std::unordered_set<ColonneNamesSet*>& usefull_col = op->Getm_Cols();

                    auto colToValList = std::make_unique<std::unordered_map<std::string, std::vector<ColumnData>*>>();

                    int nbr_ligne_mini = -1;

                    for (auto& e : usefull_col) {

                        std::unique_ptr<std::vector<ColumnData>> temp = Magasin->GetTableByName(*e->GetTableSet())->GetSampleFromColumn(*e);

                        if (nbr_ligne_mini == -1 || (*temp).size() < nbr_ligne_mini) {
                            nbr_ligne_mini = (*temp).size();
                        }

                        colToValList->insert({ e->GetMainName(), temp.get() });
                    }

                    auto CombinaisonATester = std::make_unique<std::unordered_map<std::string, ColumnData>>();

                    for (int ligne = 0; ligne < nbr_ligne_mini; ligne++) {
                        for (auto& e : usefull_col) {
                            (*CombinaisonATester)[e->GetMainName()] = (*(*colToValList)[e->GetMainName()])[ligne];
                        }

                        auto temp = cond.EstimeSelectivite(CombinaisonATester.get());
                    }

                    if (benchmarking == 0) {

                        std::cout << "\n Voici la condition brute : \n";

                        cond.PrintCondition(std::cout);

                        std::cout << "\n et maintenant optimisant la condition : \n";
                    }

                    cond.OptimiseBinaryExpression();

                    if (benchmarking == 0) {

                        cond.PrintCondition(std::cout);
                    }
                }
            } else {
                if (benchmarking == 0) {
                    std::cout << "Il y as where mais aucun select après le projecteur principal\n"
                              << std::endl; // erreur
                }
            }
        }
    }
    if (Ordering_Join == 1 and join_list.size() >= 2) { // no need to optimize if there is just one join of no join at all

        std::vector<std::pair<Join*, float>> JoinAndRCs = std::vector<std::pair<Join*, float>>();
        for (auto Join : join_list) {
            JoinAndRCs.push_back(std::make_pair(Join, Join->calculeRC(Magasin->GetTableByName(Join->GetLTable()), Magasin->GetTableByName(Join->GetRTable()), type_of_join)));
        }
        std::sort(JoinAndRCs.begin(), JoinAndRCs.end(),
            [&](std::pair<Join*, float> a, std::pair<Join*, float> b) { return a.second < b.second; });

        Node* last;
        Utils::UnionFind uf = Utils::UnionFind();
        for (auto joinandrc : JoinAndRCs) {
            if (benchmarking == 0) {

                std::cout << "Le Join entre " << joinandrc.first->GetLTable().GetMainName() << " et " << joinandrc.first->GetRTable().GetMainName() << " a une RC de :" << joinandrc.second << "\n";
            }
            last = uf.AddElem(joinandrc.first);
        }

        RacineMainTable->AddChild(true, last);
        if (benchmarking == 0) {
            std::cout << "\n en Optimisant le plan en fonction des RC on a : \n";

            RacineExec.printBT(std::cout);
        }
    }
    if (where != NULL and descend_select == 1) {
        RacineExec.SelectionDescent(Magasin, MainSelect);
        if (benchmarking == 0) {
            std::cout << "\n en descendant les sélections on a : \n";

            RacineExec.printBT(std::cout);
        }
    }
    if (InserProj == 1) {
        auto ColumnToKeep = std::make_unique<std::unordered_set<const ColonneNamesSet*>>();

        RacineExec.InsertProj(ColumnToKeep.get());

        if (benchmarking == 0) {
            std::cout << "\n en insérant des Projections là où il faut : \n";
            RacineExec.printBT(std::cout);
        }
    }

    std::chrono::high_resolution_clock::time_point fin;

    std::unique_ptr<MetaTable> Table_Finale = std::unique_ptr<MetaTable>(RacineExec.Pronf(Magasin, type_of_join));

    auto endTime = std::chrono::high_resolution_clock::now();

    if (IsAgregate || IsOrderBy || IsLimite) { // la requete possède une agregation et donc un group by
        fin = AppliqueAggr.AppliqueAgregateAndPrint(Table_Finale.get(), benchmarking);
    } else {
        fin = std::chrono::high_resolution_clock::now();

        if (benchmarking == 0) {
            Utils::AfficheResultat(std::move(Table_Finale), std::move(colonnes_de_retour));
        }
    }

    if (benchmarking == 1) {
        std::ofstream file;
        file.open("../script/data.csv", std::ios::app);
        if (!file.is_open()) {
            std::cout << "Error: File not found or could not be opened." << std::endl;
        } else {
            file << descend_select << ";" << type_of_join << ";" << InserProj << ";" << optimize_BinaryExpr << ";" << Ordering_Join << ";" << std::chrono::duration_cast<std::chrono::microseconds>(fin - deb).count() << ";" << tables_secondaires.size() << "\n";
            std::cout << "Requête parfaitement executée";
        }
        file.close();
    }
}
};
