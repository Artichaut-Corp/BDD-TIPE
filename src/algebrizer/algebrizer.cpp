// Le but ici est de transformer l'arbre former par le parser un arbre très naïf qui seras ensuite modifié par l'optimiser
#include "../algebrizer/algebrizer.h"
#include "../data_process_system/meta-table.h"
#include "../data_process_system/namingsystem.h"
#include "../parser.h"
#include "../storage.h"
#include "../utils/printing_utils.h"
#include "../utils/union_find.h"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace Database::QueryPlanning {

std::shared_ptr<ColonneNamesSet> ConvertToStandardColumnName(std::shared_ptr<TableNamesSet> NomTablePrincipale, Database::Parsing::ColumnName* Colonne, std::unordered_map<std::string, std::shared_ptr<TableNamesSet>>* variation_of_tablename_to_main_table_name)
{
    std::shared_ptr<ColonneNamesSet> StandardName = nullptr;
    if (Colonne->HaveTable()) {
        std::shared_ptr<TableNamesSet> table = (*variation_of_tablename_to_main_table_name)[Colonne->GetTable()];
        auto FullName = Colonne->getColumnName();
        StandardName = std::make_shared<ColonneNamesSet>(ColonneNamesSet(FullName, *Colonne->GetAlias(), table)); // récupere le nom de cette colonne
    } else { // la colonne n'as pas de nom de table, on en conclu que c'est une colonne de la table principale, il faut donc rajouter le nom de cette table à son identifiant
        StandardName = std::make_shared<ColonneNamesSet>(ColonneNamesSet(Colonne->getColumnName(), *Colonne->GetAlias(), NomTablePrincipale));
    }

    return StandardName;
}
std::shared_ptr<TableNamesSet> ConvertToStandardTableName(Database::Parsing::TableName* Table, std::unordered_map<std::string, std::shared_ptr<TableNamesSet>>* variation_of_tablename_to_main_table_name)
{
    std::shared_ptr<TableNamesSet> StandardName = std::make_shared<TableNamesSet>(TableNamesSet(Table->getTableName()));
    (*variation_of_tablename_to_main_table_name)[StandardName->GetMainName()] = StandardName;
    for (auto e : *Table->GetAlias()) {
        StandardName->AddAlias(e);
        (*variation_of_tablename_to_main_table_name)[e] = StandardName;
    }
    return StandardName;
}

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection, Storing::File* File, std::unordered_map<std::basic_string<char>, Database::Storing::TableInfo>* IndexGet, std::shared_ptr<std::vector<int>> param)
{
    int descend_select = (*param)[0];
    int type_of_join = (*param)[1];
    int InserProj = (*param)[2];
    int optimize_BinaryExpr = (*param)[3];
    int Ordering_Join = (*param)[4];

    // Implémentation d'une conversion en arbre d'une query simple
    std::unordered_map<std::string, std::shared_ptr<TableNamesSet>> variation_of_tablename_to_main_table_name;
    std::shared_ptr<TableNamesSet> TablePrincipaleNom = ConvertToStandardTableName(Selection->getTable(), &variation_of_tablename_to_main_table_name); // ne peut pas être nullptr
    // récupérer la liste des colonne de retour,
    std::vector<ReturnType*> colonnes_de_retour;
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<ColonneNamesSet>>> TableNameToColumnList;
    std::unordered_set<std::shared_ptr<ColonneNamesSet>> UsefullColumnForAggrAndOutput;
    bool IsAgregate = false;

    // pour les join
    std::vector<std::shared_ptr<TableNamesSet>> tables_secondaires;

    std::vector<Join*> join_list;

    std::vector<Parsing::Join>* joins = Selection->getJoins();

    if (joins != nullptr) { // si il y as des join, on suppose que ce sont tous des join classique càd des inner join
        for (const Parsing::Join& j : *joins) {

            if (j.getJoinType() == Parsing::JoinType::INNER_J) {

                tables_secondaires.push_back(ConvertToStandardTableName(j.getTable(), &variation_of_tablename_to_main_table_name));

                std::shared_ptr<ColonneNamesSet> colonne_gauche = (ConvertToStandardColumnName(TablePrincipaleNom, j.getLeftColumn(), &variation_of_tablename_to_main_table_name));
                std::shared_ptr<ColonneNamesSet> colonne_droite = (ConvertToStandardColumnName(TablePrincipaleNom, j.getRightColumn(), &variation_of_tablename_to_main_table_name));

                TableNameToColumnList[colonne_gauche->GetTableSet()->GetMainName()].emplace(colonne_gauche);

                TableNameToColumnList[colonne_droite->GetTableSet()->GetMainName()].emplace(colonne_droite);

                Comparateur condition = Comparateur(Parsing::LogicalOperator::EQ); // dans tout les cas c'est un égal

                Join* jointure = new Join(condition, colonne_gauche, colonne_droite);

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
                    auto NomColonne = (ConvertToStandardColumnName(TablePrincipaleNom, &(arg.m_Field.value()), &variation_of_tablename_to_main_table_name));
                    UsefullColumnForAggrAndOutput.emplace(NomColonne);
                    TableNameToColumnList[NomColonne->GetTableSet()->GetMainName()].emplace(NomColonne);
                    colonnes_de_retour.push_back(new ReturnType(NomColonne, Parsing::AggrFuncType::NOTHING_F));

                } else {
                    // bizare, c'est normalement impossible
                }
            }
        } else if (std::holds_alternative<Parsing::AggregateFunction>(colonne_info)) { // est une fonction d'agrégation
            auto arg = std::get<Parsing::AggregateFunction>(colonne_info);
            if (!arg.isAll()) {
                // on vérifie que y'as bien une valeur, c'est un type optinal
                std::shared_ptr<ColonneNamesSet> NomColonne = ConvertToStandardColumnName(TablePrincipaleNom, arg.getColumnName(), &variation_of_tablename_to_main_table_name);
                TableNameToColumnList[NomColonne->GetTableSet()->GetMainName()].emplace(NomColonne);
                colonnes_de_retour.push_back(new ReturnType(NomColonne, arg.getType()));
                IsAgregate = true;
                UsefullColumnForAggrAndOutput.emplace(NomColonne);

            } else {
                std::cout << "y'as une étoile\n"
                          << std::endl; // erreur
            }
        } else {
            std::cout << "type inconu parmis m_Fields lors de la création des colonnes de retour\n"
                      << std::endl; // erreur
        }
    }

    Final AppliqueAggr(&colonnes_de_retour);
    if (IsAgregate) { // permet de créer les agrégation si il y en as
        Parsing::GroupByClause* Groupby = Selection->getGroupBy();
        if (Groupby != nullptr) {
            std::vector<std::shared_ptr<ColonneNamesSet>>* ColumnGroupByed = new std::vector<std::shared_ptr<ColonneNamesSet>>;
            for (auto e : Groupby->getByItems()) {
                std::shared_ptr<ColonneNamesSet> NomColonne = ConvertToStandardColumnName(TablePrincipaleNom, e.getColName(), &variation_of_tablename_to_main_table_name);
                TableNameToColumnList[NomColonne->GetTableSet()->GetMainName()].emplace(NomColonne);
                ColumnGroupByed->push_back(NomColonne);
                UsefullColumnForAggrAndOutput.emplace(NomColonne);
            }
            AppliqueAggr.AjouteGroupBy(ColumnGroupByed);
        }
    }

    // permet de créer les OrderBy si il y en as
    Parsing::OrderByClause* order = Selection->getOrderBy();
    bool IsOrderBy = false;
    if (order != nullptr) {
        IsOrderBy = true;
        std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>>* OrderVect = new std::vector<std::pair<std::shared_ptr<ColonneNamesSet>, bool>>;
        for (auto e : order->getByItems()) {
            std::shared_ptr<ColonneNamesSet> NomColonne = ConvertToStandardColumnName(TablePrincipaleNom, e.getColName(), &variation_of_tablename_to_main_table_name);
            bool est_présent = false;
            for (auto x : colonnes_de_retour) {
                if (*x->GetColonne() == *NomColonne) {
                    est_présent = true;
                }
            }
            if (!est_présent) { // évite les doublons dans la projection finale et dans la création des tables
                TableNameToColumnList[NomColonne->GetTableSet()->GetMainName()].emplace(NomColonne);
                UsefullColumnForAggrAndOutput.emplace(NomColonne);
            }
            OrderVect->push_back(std::pair<std::shared_ptr<ColonneNamesSet>, bool>(NomColonne, (!e.isDsc()))); // on inverse le Desc car il est vrai si c'est inversé et dans la suite on considère que si c'est vrai alors c'est Asc
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
    std::unordered_set<std::shared_ptr<ColonneNamesSet>>* ConditionColumn;
    Parsing::BinaryExpression::Condition Condition; // those variable are used two times,

    if (where != nullptr) { // il faut ajouter les colonnes utilisé dans la conditions avant de créer la table principale
        std::unordered_set<std::shared_ptr<ColonneNamesSet>>* ColonneTesté;

        Condition = where->m_Condition;

        if (std::holds_alternative<Parsing::BinaryExpression*>(Condition)) {
            std::get<Parsing::BinaryExpression*>(Condition)->FormatColumnName(TablePrincipaleNom);
            ConditionColumn = std::get<Parsing::BinaryExpression*>(Condition)->Column();
        } else {
            std::get<Parsing::Clause*>(Condition)->FormatColumnName(TablePrincipaleNom);
            ConditionColumn = std::get<Parsing::Clause*>(Condition)->Column();
        }
        for (auto NomColonne : *ConditionColumn) {
            bool est_présent = false;
            for (auto x : colonnes_de_retour) {
                if (*x->GetColonne() == *NomColonne) {
                    est_présent = true;
                }
            }
            if (!est_présent) { // évite les doublons dans la projection finale et dans la création des tables
                TableNameToColumnList[NomColonne->GetTableSet()->GetMainName()].emplace(NomColonne);
            }
        }
    }

    //  on doit creer la table principale, pour cela on doit creer les racines et les Colonnes
    std::vector<std::shared_ptr<Racine>> Racines;
    Racines.reserve(TableNameToColumnList[TablePrincipaleNom->GetMainName()].size());
    std::unordered_set<std::shared_ptr<ColonneNamesSet>> ColonneAlreadyCreate;
    for (std::shared_ptr<ColonneNamesSet> colonne_nom : TableNameToColumnList[TablePrincipaleNom->GetMainName()]) {
        bool est_déjà_ajouté = false;
        for (auto e : ColonneAlreadyCreate) {
            if (*colonne_nom == *e) {
                est_déjà_ajouté = true;
                colonne_nom->FusionColumn(e);
                break;
            }
        }
        if (!est_déjà_ajouté) {
            std::shared_ptr<Racine> RacinePtr = std::make_shared<Racine>(Racine(colonne_nom, File->Fd(), IndexGet));
            Racines.push_back(RacinePtr);
            ColonneAlreadyCreate.emplace(colonne_nom);
        }
    }
    // Maintenant que l'on as tout pour la table Principale on la créer
    std::shared_ptr<MetaTable> table_principale = std::make_shared<MetaTable>(MetaTable(std::make_shared<Table>(Table(&Racines, TablePrincipaleNom))));
    Node RacineExec = Node(new Proj(&UsefullColumnForAggrAndOutput, TablePrincipaleNom)); // le tout dernier élément vérifie que les valeur restante sont celle de retour, donc on projete sur le type de retour
    std::vector<MetaTable> Tables;
    Tables.push_back(*table_principale); // on enregiste la table principale
    auto RacineMainTable = &RacineExec;

    std::unordered_map<std::string, std::pair<Node*, bool>> TableToRootOfTableMap; // envoie l'endroit du plus petit noeud dans le plan d'éxécution où cette table est attendu (le booléen est là pour savoir si en cas de join, la table est le nom de droite ou de gauche)
    TableToRootOfTableMap[TablePrincipaleNom->GetMainName()] = std::pair<Node*, bool>((&RacineExec), true);

    // il faut maintenant récupérer les conditions càd les where
    if (where != NULL) { // une foit la racine de l'arbre d'éxécution définie, on peut lui ajouter une selection si nécessaire
        MainSelect = new Select(std::make_unique<std::unordered_set<std::shared_ptr<ColonneNamesSet>>>(*ConditionColumn), Condition, TablePrincipaleNom);
        Node* Node_Select = new Node(MainSelect);
        RacineExec.AddChild(true, Node_Select);
        TableToRootOfTableMap[TablePrincipaleNom->GetMainName()] = std::pair<Node*, bool>(Node_Select, true);
        RacineMainTable = Node_Select;
    }

    if (!tables_secondaires.empty()) { // si il y as des join
        //  on doit creer les autres tables
        //  pour cela on vas créer les racines et les colonne et donc les tables de chaque sous-table avant de créer l'arbre

        for (int i = 0; i < tables_secondaires.size(); i++) {
            std::vector<std::shared_ptr<Racine>> Racines;
            Racines.reserve(TableNameToColumnList[tables_secondaires[i]->GetMainName()].size());
            ColonneAlreadyCreate.clear();

            for (std::shared_ptr<ColonneNamesSet> colonne_nom : TableNameToColumnList[tables_secondaires[i]->GetMainName()]) {
                bool est_déjà_ajouté = false;
                for (auto e : ColonneAlreadyCreate) {
                    if (*colonne_nom == *e) {
                        est_déjà_ajouté = true;
                        colonne_nom->FusionColumn(e);
                        break;
                    }
                }
                if (!est_déjà_ajouté) {
                    std::shared_ptr<Racine> RacinePtr = std::make_shared<Racine>(Racine(colonne_nom, File->Fd(), IndexGet));
                    Racines.push_back(RacinePtr);
                    ColonneAlreadyCreate.emplace(colonne_nom);
                }
            }
            // Maintenant que l'on as tout pour la table Principale on la créer
            std::shared_ptr<MetaTable> table_secondaire = std::make_shared<MetaTable>(MetaTable(std::make_shared<Table>(Table(&Racines, tables_secondaires[i]))));
            Tables.push_back(*table_secondaire);
            std::shared_ptr<TableNamesSet> TableDéjàAjouter = nullptr; // dans chaque création de jointure,il y a déjà une table présente dans l'arbre d'éxécution
            std::shared_ptr<TableNamesSet> TableGauche = join_list[i]->GetLTable();
            std::shared_ptr<TableNamesSet> TableDroite = join_list[i]->GetRTable();
            if (TableToRootOfTableMap.contains(TableGauche->GetMainName())) {
                TableDéjàAjouter = TableGauche;
            } else {
                TableDéjàAjouter = TableDroite;
            }
            Node* NoeudRacineTableDéjàAjouter = std::move(TableToRootOfTableMap[TableDéjàAjouter->GetMainName()].first);
            bool EstGauche = TableToRootOfTableMap[TableDéjàAjouter->GetMainName()].second;
            Node* EmplacementNouveauJoin = new Node(join_list[i]);
            NoeudRacineTableDéjàAjouter->AddChild(EstGauche, EmplacementNouveauJoin);
            TableToRootOfTableMap[TableDroite->GetMainName()] = std::pair<Node*, bool>(EmplacementNouveauJoin, false);
            TableToRootOfTableMap[TableGauche->GetMainName()] = std::pair<Node*, bool>(EmplacementNouveauJoin, true);
        }
    }
    Ikea* Magasin = new Ikea(Tables);
    RacineExec.printBT(std::cout);
    if (where != NULL and optimize_BinaryExpr == 1) {
        auto SelectNode = RacineExec.GetLeftPtr();
        if (SelectNode == nullptr) {
            std::cout << "Absurdité, where n'est pas null mais aucun select n'est présent\n"
                      << std::endl; // erreur
        } else {
            auto SelectAct = SelectNode->GetAction();
            if (std::holds_alternative<Select*>(SelectAct)) {
                Select* op = std::get<Select*>(SelectAct);
                if (std::holds_alternative<Parsing::BinaryExpression*>(op->GetCond())) { // if the cond is a clause or a tautology, we can't otpimize it
                    Parsing::BinaryExpression* cond = std::get<Database::Parsing::BinaryExpression*>(op->GetCond());
                    auto usefull_col = op->Getm_Cols();
                    std::unordered_map<std::string, std::vector<ColumnData>*>* colToValList = new std::unordered_map<std::string, std::vector<ColumnData>*>();
                    int nbr_ligne_mini = -1;
                    for (auto e : *usefull_col) {
                        auto temp = Magasin->GetTableByName(e->GetTableSet())->GetSample(e);
                        if (nbr_ligne_mini == -1 || (*temp).size() < nbr_ligne_mini) {
                            nbr_ligne_mini = (*temp).size();
                        }
                        (*colToValList)[e->GetMainName()] = temp;
                    }

                    std::unordered_map<std::string, ColumnData>* CombinaisonATester = new std::unordered_map<std::string, ColumnData>();
                    for (int ligne = 0; ligne < nbr_ligne_mini; ligne++) {
                        for (auto e : *usefull_col) {
                            (*CombinaisonATester)[e->GetMainName()] = (*(*colToValList)[e->GetMainName()])[ligne];
                        }
                        auto temp = cond->EstimeSelectivite(CombinaisonATester);
                    }

                    std::cout << "\n Voici la condition brute : \n";

                    cond->PrintCondition(std::cout);

                    std::cout << "\n et maintenant optimisant la condition : \n";
                    cond->OptimiseBinaryExpression();
                    cond->PrintCondition(std::cout);
                }
            } else {
                std::cout << "Il y as where mais aucun select après le projecteur principal\n"
                          << std::endl; // erreur
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
            std::cout << "Le Join entre " << joinandrc.first->GetLTable()->GetMainName() << " et " << joinandrc.first->GetRTable()->GetMainName() << " a une RC de :" << joinandrc.second << "\n";

            last = uf.AddElem(joinandrc.first);
        }

        RacineMainTable->AddChild(true, last);
        std::cout << "\n en Optimisant le plan en fonction des RC on a : \n";
        RacineExec.printBT(std::cout);
    }
    if (where != NULL and descend_select == 1) {
        std::cout << "\n en descendant les sélections on a : \n";
        RacineExec.SelectionDescent(Magasin, MainSelect);
        RacineExec.printBT(std::cout);
    }
    if (InserProj == 1) {
        std::cout << "\n en insérant des Projections là où il faut : \n";
        std::unordered_set<std::shared_ptr<ColonneNamesSet>>* ColumnToKeep = new std::unordered_set<std::shared_ptr<ColonneNamesSet>>;
        RacineExec.InsertProj(ColumnToKeep);
        RacineExec.printBT(std::cout);
    }
    std::shared_ptr<MetaTable> Table_Finale = RacineExec.Pronf(Magasin, type_of_join);
    if (IsAgregate || IsOrderBy || IsLimite) { // la requete possède une agregation et donc un group by
        AppliqueAggr.AppliqueAgregateAndPrint(Table_Finale);
    } else {

        Utils::AfficheResultat(Table_Finale, &colonnes_de_retour);
    }
}
};
