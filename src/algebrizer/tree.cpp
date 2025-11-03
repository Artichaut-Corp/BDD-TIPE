#include "tree.h"
#include "../data_process_system/namingsystem.h"
#include "../data_process_system/table.h"
#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"
#include "../parser/expression.h"
#include "../utils/unordered_set_utils.h"
#include "ikea.h"
#include <unordered_set>
#include <variant>

namespace Database::QueryPlanning {
using NodeType = std::variant<Join*, Proj*, Select*>; // le type root est censé être la racine de la query et ne jamais parti de là

Table* Node::Pronf(Ikea* Tables) // parcours en profondeur pour calculer le résultat de l'arbre d'éxécution
{
    Table* result = nullptr;

    Table* tFg = nullptr;
    Table* tFd = nullptr;

    if (m_Fg) {
        tFg = m_Fg->Pronf(Tables);
    }
    if (m_Fd) {
        tFd = m_Fd->Pronf(Tables);
    }
    int JoinParam = 3; // allow us to chose which join use
    if (std::holds_alternative<Join*>(m_Type)) {
        auto op = std::get<Join*>(m_Type);
        if (!tFd) {
            tFd = Tables->GetTableByName(op->GetRTable());
        }
        if (!tFg) {
            tFg = Tables->GetTableByName(op->GetLTable());
        }
        if (JoinParam == 0) {
            result = op->ExecNaif(tFg, tFd);
        } else if (JoinParam == 1) {
            result = op->ExecTrier(tFg, tFd);
        } // else if (JoinParam == 2    ) {
        //    result = op->ExecTrierStockerMemoire(tFg, tFd);
        //}
        else if (JoinParam == 3) {
            result = op->ExecGrouByStyle(tFg, tFd);
        }
        else {
            throw std::runtime_error("Type de Join Inconnu");
        }
    } else if (std::holds_alternative<Proj*>(m_Type)) {
        auto op = std::get<Proj*>(m_Type);

        if (tFg) {
            result = op->Exec(tFg); // doit retourner Table*
        } else {
            result = op->Exec(Tables->GetTableByName(op->GetTableName()));
        }
    } else if (std::holds_alternative<Select*>(m_Type)) {
        auto op = std::get<Select*>(m_Type);

        if (tFg) {
            result = op->Exec(tFg); // doit retourner Table*
        } else {
            result = op->Exec(Tables->GetTableByName(op->GetTableName()));
        }
    } else {
        throw std::runtime_error("Unknown node type");
    }
    return result;
}

void Node::printBT(const std::string& prefix, const Node* node, bool isLeft, std::ostream& out)
{
    if (node != nullptr) {
        out << prefix;

        out << (isLeft && node->m_Fd && std::holds_alternative<Join*>(node->m_Fd->m_Type) ? "├──" : "└──");

        auto type = node->m_Type;

        if (std::holds_alternative<Join*>(type)) {
            Join* op = std::get<Join*>(type);
            out << "Jointure entre la table " << op->GetLTable()->GetMainName() << " et " << op->GetRTable()->GetMainName() << " sur " << op->GetLCol()->GetMainName() << " " << op->GetComp().GetLO() << " " << op->GetRCol()->GetMainName() << "\n";

        } else if (std::holds_alternative<Proj*>(type)) {
            Proj* op = std::get<Proj*>(type);
            out << "Projection sur les colonnes : ";
            bool début = true;
            for (auto e : *op->Getm_Cols()) {
                if (!début) {
                    out << " et ";
                }
                out << e->GetMainName();
                début = false;
            }
            out << "\n";

        } else if (std::holds_alternative<Select*>(type)) {
            Select* op = std::get<Select*>(type);
            out << "Selection sur " << op->GetTableName()->GetMainName() << " avec : ";

            auto cond = op->GetCond();
            if (std::holds_alternative<Parsing::BinaryExpression*>(cond)) {
                Parsing::BinaryExpression* be = std::get<Parsing::BinaryExpression*>(cond);
                if (be) {
                    be->PrintCondition(out);
                } else {
                    out << "[BinaryExpression* null] \n";
                }
            } else if (std::holds_alternative<Parsing::Clause*>(cond)) {
                std::get<Parsing::Clause*>(cond)->Print(out);
                out << "\n";
            } else {
                out << "Tautologie" << "\n";
            }

        } else {
            out << "Type inconnu\n";
        }

        // enter the next tree level - left and right branch
        if (node->m_Fg) {
            printBT(prefix + (isLeft && node->m_Fd && std::holds_alternative<Join*>(node->m_Fd->m_Type) ? "│   " : "    "), node->m_Fg, true, out);
        }
        if (node->m_Fd) {
            printBT(prefix + (isLeft && node->m_Fd && std::holds_alternative<Join*>(node->m_Fd->m_Type) ? "│   " : "    "), node->m_Fd, false, out);
        }
    }
}
void Node::printBT(std::ostream& out)
{
    printBT("", this, false, out);
}

std::unordered_set<ColonneNamesSet*>* Node::SelectionDescent(Ikea* Tables, Select* MainSelect)
{
    if (!std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si c'est vrai la condition a déjà été descendu et toute descente est inutile
        std::unordered_set<ColonneNamesSet*>* SFg = nullptr;
        std::unordered_set<ColonneNamesSet*>* SFd = nullptr;
        if (m_Fg) {
            SFg = m_Fg->SelectionDescent(Tables, MainSelect); // appelle récursif, voire la suite du code

            if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join

                auto jointure = std::get<Join*>(m_Type);

                std::unordered_set<ColonneNamesSet*>* ColumnInCond; // identification des colonnes encore existante dans la condition
                auto MainCond = MainSelect->GetCond();
                if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                    ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                } else {
                    ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                }

                if (Utils::is_subset(ColumnInCond, SFg)) { // on peut tout mettre en bas à gauche
                    MainSelect->NullifyCond();
                    auto temp = m_Fg;
                    m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                    m_Fg->AddChild(true, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                    return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                } else { // on découpe MainCond
                    Parsing::BinaryExpression::Condition RecupGauche;
                    std::unordered_set<ColonneNamesSet*>* ColumnUsedInCondGauche;
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        RecupGauche = std::monostate {}; // on ne peut pas couper une clause
                        ColumnUsedInCondGauche = {};
                    } else {
                        RecupGauche = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                        if (std::holds_alternative<std::monostate>(RecupGauche)) {
                            ColumnUsedInCondGauche = {};
                        } else if (std::holds_alternative<Parsing::Clause*>(RecupGauche)) {
                            ColumnUsedInCondGauche = std::get<Parsing::Clause*>(RecupGauche)->Column();
                        } else {
                            ColumnUsedInCondGauche = std::get<Parsing::BinaryExpression*>(RecupGauche)->Column();
                        }
                    }

                    if (!std::holds_alternative<std::monostate>(RecupGauche)) { // ce qu'on as extrait n'est pas vide
                        auto temp = m_Fg;
                        m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnUsedInCondGauche), RecupGauche, jointure->GetLTable()));
                        m_Fg->AddChild(true, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                    }
                }
            } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
        }
        if (m_Fd && !std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si l'appel récursif à gauche ne l'as pas vidé
            SFd = m_Fd->SelectionDescent(Tables, MainSelect);
            if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join
                auto jointure = std::get<Join*>(m_Type);

                std::unordered_set<ColonneNamesSet*>* ColumnInCond; // identification des colonnes encore existante dans la condition
                auto MainCond = MainSelect->GetCond();
                if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                    ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                } else {
                    ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                }

                if (Utils::is_subset(ColumnInCond, SFd)) { // on peut tout mettre en bas à droite
                    MainSelect->NullifyCond();
                    auto temp = m_Fd;
                    m_Fd = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                    m_Fd->AddChild(false, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                    return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                } else { // on découpe MainCond
                    Parsing::BinaryExpression::Condition RecupDroit;
                    std::unordered_set<ColonneNamesSet*>* ColumnUsedInCondDroit;
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        RecupDroit = std::monostate {}; // on ne peut pas couper une clause
                        ColumnUsedInCondDroit = {};
                    } else {
                        RecupDroit = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                        if (std::holds_alternative<std::monostate>(RecupDroit)) {
                            ColumnUsedInCondDroit = {};
                        } else if (std::holds_alternative<Parsing::Clause*>(RecupDroit)) {
                            ColumnUsedInCondDroit = std::get<Parsing::Clause*>(RecupDroit)->Column();
                        } else {
                            ColumnUsedInCondDroit = std::get<Parsing::BinaryExpression*>(RecupDroit)->Column();
                        }
                    }

                    if (!std::holds_alternative<std::monostate>(RecupDroit)) { // ce qu'on as extrait n'est pas vide
                        auto temp = m_Fg;
                        m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnUsedInCondDroit), RecupDroit, jointure->GetLTable()));
                        m_Fg->AddChild(true, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                    }
                }
            } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
        }

        if (!std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si les appels récursif à droite et à gauche ne l'as pas vidé
            if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join
                auto jointure = std::get<Join*>(m_Type);
                if (!m_Fg) { // si ce join n'as rien à gauche, on essaie de mettre un select en dessous à gauche du join
                    SFg = new std::unordered_set<ColonneNamesSet*>;
                    auto ColonneDispoGauche = Tables->GetTableByName(jointure->GetLTable())->GetColumnNames();
                    SFg->insert(ColonneDispoGauche->begin(), ColonneDispoGauche->end());
                    std::unordered_set<ColonneNamesSet*>* ColumnInCond; // on récupère les colonnes présente dans la conditions
                    auto MainCond = MainSelect->GetCond();
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                    } else { // non std::monostate par condition d'entrée
                        ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                    }
                    if (Utils::is_subset(ColumnInCond, SFg)) { // on peut tout mettre en bas à gauche
                        MainSelect->NullifyCond();
                        m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                        return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                    } else { // on découpe MainCond
                        Parsing::BinaryExpression::Condition RecupGauche;
                        std::unordered_set<ColonneNamesSet*>* ColumnUsedInCondGauche;
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            RecupGauche = std::monostate {}; // on ne peut pas couper une clause
                            ColumnUsedInCondGauche = {};
                        } else {
                            RecupGauche = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                            if (std::holds_alternative<std::monostate>(RecupGauche)) {
                                ColumnUsedInCondGauche = {};
                            } else if (std::holds_alternative<Parsing::Clause*>(RecupGauche)) {
                                ColumnUsedInCondGauche = std::get<Parsing::Clause*>(RecupGauche)->Column();
                            } else {
                                ColumnUsedInCondGauche = std::get<Parsing::BinaryExpression*>(RecupGauche)->Column();
                            }
                        }

                        if (!std::holds_alternative<std::monostate>(RecupGauche)) { // ce qu'on as extrait n'est pas vide
                            m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnUsedInCondGauche), RecupGauche, jointure->GetLTable()));
                        }
                    }
                }
                // on fait la même chose à droite
                if (!m_Fd && !std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si ce join n'as rien à droite et que la condtion n'est plus vide, on essaie de mettre un select en dessous à droite du join
                    SFd = new std::unordered_set<ColonneNamesSet*>;

                    auto ColonneDispoDroite = Tables->GetTableByName(jointure->GetRTable())->GetColumnNames();
                    std::copy(ColonneDispoDroite->begin(), ColonneDispoDroite->end(), std::inserter(*SFd, SFd->end())); // on récupere les colonnes dispo à droite

                    std::unordered_set<ColonneNamesSet*>* ColumnInCond; // on récupère les colonnes présente dans la conditions
                    auto MainCond = MainSelect->GetCond();
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                    } else {
                        ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                    }

                    if (Utils::is_subset(ColumnInCond, SFd)) { // on peut tout mettre en bas à droite
                        MainSelect->NullifyCond();
                        m_Fd = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                        return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                    } else { // on découpe MainCond
                        Parsing::BinaryExpression::Condition RecupDroit;
                        std::unordered_set<ColonneNamesSet*>* ColumnUsedInCondDroit;
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            RecupDroit = std::monostate {}; // on ne peut pas couper une clause
                            ColumnUsedInCondDroit = {};
                        } else {
                            RecupDroit = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                            if (std::holds_alternative<std::monostate>(RecupDroit)) {
                                ColumnUsedInCondDroit = {};
                            } else if (std::holds_alternative<Parsing::Clause*>(RecupDroit)) {
                                ColumnUsedInCondDroit = std::get<Parsing::Clause*>(RecupDroit)->Column();
                            } else {
                                ColumnUsedInCondDroit = std::get<Parsing::BinaryExpression*>(RecupDroit)->Column();
                            }
                        }

                        if (!std::holds_alternative<std::monostate>(RecupDroit)) { // ce qu'on as extrait n'est pas vide
                            m_Fg = new Node(new Select(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*ColumnUsedInCondDroit), RecupDroit, jointure->GetLTable()));
                        }
                    }
                }
                // ici on a défini SFg et SFd si jamais ils ne l'était pas, on vas donc les joindre et renvoyer leur union en espérant que le parent vas pouvoir mettre une selection entre cette jointure et lui même
                SFg->insert(SFd->begin(), SFd->end());
                return SFg;
            } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
        }
        if (SFg != nullptr) {
            if (SFd != nullptr) {
                SFg->insert(SFd->begin(), SFd->end());
                return SFg;
            } else {
                return SFg;
            }
        } else {
            if (SFd != nullptr) {
                return SFd;
            } else {
                return nullptr;
            }
        }

    } else {
        return nullptr; // on remonte l'arbre
    }
}
};
