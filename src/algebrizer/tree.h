#include "../data_process_system/table.h"
#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"
#include "../parser/expression.h"
#include "../utils/unordered_set_utils.h"
#include "ikea.h"
#include <unordered_set>
#include <variant>

#ifndef TREE_H

#define TREE_H

namespace Database::QueryPlanning {
using NodeType = std::variant<Join*, Proj*, Select*>; // le type root est censé être la racine de la query et ne jamais parti de là

class Node {
private:
    NodeType m_Type;
    Node* m_Fg = nullptr;
    Node* m_Fd = nullptr;

public:
    Node(NodeType type)
        : m_Type(type)
    {
    }

    void AddChild(bool left, Node* child)
    {
        if (left)
            m_Fg = child;
        else
            m_Fd = child;
    }

    Table* Pronf(Ikea* Tables)
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
        result = std::visit([&](auto* op) -> Table* { // <-- retour explicite Table*
            using T = std::decay_t<decltype(*op)>;
            if constexpr (std::is_same_v<T, Join>) {
                if (tFg && tFd) {
                    return op->Exec(tFg, tFd); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetLTable()), Tables->GetTableByName(op->GetRTable()));
                }
            } else if constexpr (std::is_same_v<T, Proj>) {
                if (tFg) {
                    return op->Exec(tFg); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetTableName()));
                }
            } else if constexpr (std::is_same_v<T, Select>) {
                if (tFg) {
                    return op->Exec(tFg); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetTableName()));
                }
            } else {
                throw std::runtime_error("Unknown node type");
            }
        },
            m_Type);

        return result;
    }

    void printBT(const std::string& prefix, const Node* node, bool isLeft, std::ostream& out)
    {
        if (node != nullptr) {
            out << prefix;

            out << (isLeft  ? "├──" : "└──");

            auto type = node->m_Type;

            if (std::holds_alternative<Join*>(type)) {
                Join* op = std::get<Join*>(type);
                out << "Jointure entre la table " << op->GetLTable() << " et " << op->GetRTable() << " sur " << op->GetLCol() << " " << op->GetComp().GetLO() << " " << op->GetRCol() << "\n";

            } else if (std::holds_alternative<Proj*>(type)) {
                Proj* op = std::get<Proj*>(type);
                out << "Projection sur " << op->GetTableName() << "\n";

            } else if (std::holds_alternative<Select*>(type)) {
                Select* op = std::get<Select*>(type);
                out << "Selection sur " << op->GetTableName() << " avec : ";

                // auto cond = op->GetCond();
                // if (std::holds_alternative<Parsing::BinaryExpression*>(cond)) {
                //     Parsing::BinaryExpression* be = std::get<Parsing::BinaryExpression*>(cond);
                //     if (be) {
                //         be->PrintCondition(out);
                //     } else {
                //         out << "[BinaryExpression* null]";
                //     }

                // } else if (std::holds_alternative<Parsing::Clause*>(cond)) {
                //     Parsing::Clause* c = std::get<Parsing::Clause*>(cond);

                //     if (c) {

                //         c->Print(out);

                //     } else {
                //         out << "[Clause* null]";
                //     }

                // } else {
                //     out << "Tautologie";
                // }

                out << "\n";

            } else {
                out << "Type inconnu\n";
            }

            // enter the next tree level - left and right branch
            if (node->m_Fg) {
                printBT(prefix + (isLeft ? "│   " : "    "), node->m_Fg, true, out);
            }
            if (node->m_Fd) {
                printBT(prefix + (isLeft ? "│   " : "    "), node->m_Fd, false, out);
            }
        }
    }

    void printBT(std::ostream& out)
    {
        printBT("", this, false, out);
    }

    std::unordered_set<std::string>* SelectionDescent(Ikea* Tables, Select* MainSelect)
    {
        if (!std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si c'est vrai la condition a déjà été descendu et toute descente est inutile
            std::unordered_set<std::string>* SFg = new std::unordered_set<std::string>();
            std::unordered_set<std::string>* SFd = new std::unordered_set<std::string>();
            if (m_Fg) {
                SFg = m_Fg->SelectionDescent(Tables, MainSelect); // appelle récursif, voire la suite du code

                if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join

                    auto jointure = std::get<Join*>(m_Type);

                    std::unordered_set<std::string>* ColumnInCond; // identification des colonnes encore existante dans la condition
                    auto MainCond = MainSelect->GetCond();
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                    } else {
                        ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                    }

                    if (Utils::is_subset(ColumnInCond, SFg)) { // on peut tout mettre en bas à gauche
                        MainSelect->NullifyCond();
                        auto temp = m_Fg;
                        m_Fg = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                        m_Fg->AddChild(true, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                        return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                    } else { // on découpe MainCond
                        Parsing::BinaryExpression::Condition RecupGauche;
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            RecupGauche = std::monostate {}; // on ne peut pas couper une clause
                        } else {
                            RecupGauche = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                        }

                        if (!std::holds_alternative<std::monostate>(MainCond)) { // ce qu'on as extrait n'est pas vide
                            auto temp = m_Fg;
                            m_Fg = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                            m_Fg->AddChild(true, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                        }
                    }
                } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
            }
            if (m_Fd && !std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si l'appel récursif à gauche ne l'as pas vidé
                SFd = m_Fd->SelectionDescent(Tables, MainSelect);
                if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join
                    auto jointure = std::get<Join*>(m_Type);

                    std::unordered_set<std::string>* ColumnInCond; // identification des colonnes encore existante dans la condition
                    auto MainCond = MainSelect->GetCond();
                    if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                        ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                    } else {
                        ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                    }

                    if (Utils::is_subset(ColumnInCond, SFd)) { // on peut tout mettre en bas à droite
                        MainSelect->NullifyCond();
                        auto temp = m_Fd;
                        m_Fd = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                        m_Fd->AddChild(false, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                        return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                    } else { // on découpe MainCond
                        Parsing::BinaryExpression::Condition RecupDroite;
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            RecupDroite = std::monostate {}; // on ne peut pas couper une clause
                        } else {
                            RecupDroite = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFd);
                        }

                        if (!std::holds_alternative<std::monostate>(MainCond)) { // ce qu'on as extrait n'est pas vide
                            auto temp = m_Fd;
                            m_Fd = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                            m_Fd->AddChild(false, temp); // on insère la selection entre ce noeud, et le noeud d'en dessous
                        }
                    }
                } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
            }

            if (!std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si les appels récursif à droite et à gauche ne l'as pas vidé
                if (std::holds_alternative<Join*>(m_Type)) { // si on est sur un noeud join
                    auto jointure = std::get<Join*>(m_Type);
                    if (!m_Fg) { // si ce join n'as rien à gauche, on essaie de mettre un select en dessous à gauche du join

                        auto ColonneDispoGauche = Tables->GetTableByName(jointure->GetLTable())->GetColumnNames();
                        SFg->insert(ColonneDispoGauche->begin(), ColonneDispoGauche->end());
                        std::unordered_set<std::string>* ColumnInCond; // on récupère les colonnes présente dans la conditions
                        auto MainCond = MainSelect->GetCond();
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                        } else {
                            ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                        }

                        if (Utils::is_subset(ColumnInCond, SFg)) { // on peut tout mettre en bas à gauche
                            MainSelect->NullifyCond();
                            m_Fg = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                            return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                        } else { // on découpe MainCond
                            Parsing::BinaryExpression::Condition RecupGauche;
                            if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                                RecupGauche = std::monostate {}; // on ne peut pas couper une clause
                            } else {
                                RecupGauche = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFg);
                            }

                            if (!std::holds_alternative<std::monostate>(MainCond)) { // ce qu'on as extrait n'est pas vide
                                m_Fg = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetLTable()));
                            }
                        }
                    }
                    // on fait la même chose à droite
                    if (!m_Fd && !std::holds_alternative<std::monostate>(MainSelect->GetCond())) { // si ce join n'as rien à droite et que la condtion n'est plus vide, on essaie de mettre un select en dessous à droite du join

                        auto ColonneDispoDroite = Tables->GetTableByName(jointure->GetRTable())->GetColumnNames();
                        std::copy(ColonneDispoDroite->begin(), ColonneDispoDroite->end(), std::inserter(*SFd, SFd->end())); // on récupere les colonnes dispo à droite

                        std::unordered_set<std::string>* ColumnInCond; // on récupère les colonnes présente dans la conditions
                        auto MainCond = MainSelect->GetCond();
                        if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                            ColumnInCond = std::get<Parsing::Clause*>(MainCond)->Column();
                        } else {
                            ColumnInCond = std::get<Parsing::BinaryExpression*>(MainCond)->Column();
                        }

                        if (Utils::is_subset(ColumnInCond, SFd)) { // on peut tout mettre en bas à droite
                            MainSelect->NullifyCond();
                            m_Fd = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                            return nullptr; // on a descendu la condition entierrement, on remonte l'arbre
                        } else { // on découpe MainCond
                            Parsing::BinaryExpression::Condition RecupGauche;
                            if (std::holds_alternative<Parsing::Clause*>(MainCond)) {
                                RecupGauche = std::monostate {}; // on ne peut pas couper une clause
                            } else {
                                RecupGauche = std::get<Parsing::BinaryExpression*>(MainCond)->ExtraireCond(SFd);
                            }

                            if (!std::holds_alternative<std::monostate>(MainCond)) { // ce qu'on as extrait n'est pas vide
                                m_Fd = new Node(new Select(std::make_unique<std::unordered_set<std::string>>(*ColumnInCond), MainCond, jointure->GetRTable()));
                            }
                        }
                    }
                    // ici on a défini SFg et SFd si jamais ils ne l'était pas, on vas donc les joindre et renvoyer leur union en espérant que le parent vas pouvoir mettre une selection entre cette jointure et lui même
                    SFg->insert(SFd->begin(), SFd->end());
                    return SFg;
                } // D'après l'endroit (dans le code) où on appelle SelectionDescent, l'arbre d'apelle est de la forme P->S->J->*, ainsi si l'on n'est pas sur un join tout tentative de descente de selection est vide de sens
            }
            SFg->insert(SFd->begin(), SFd->end());
            return SFg;

        } else {
            return nullptr; // on remonte l'arbre
        }
    };
};
}
#endif // ! TREE_H