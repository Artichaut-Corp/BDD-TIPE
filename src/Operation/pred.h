#include <iostream>
#include <vector>
#include "../parser.h"

#include "../algebrizer/type def.h"

namespace Database::Querying {

using namespace Database::Querying;
class Comparateur { // permet de comparer deux elements
private:
    Database::Parsing::LogicalOperator m_Type;

public:
    Comparateur(Parsing::LogicalOperator type)
        : m_Type(type)
    {
    }

    bool Eval(Database::Querying::ColumnData val1, ColumnData val2)
    {
        if (m_Type == Parsing::LogicalOperator::EQ)
            return val1 == val2;
        if (m_Type == Parsing::LogicalOperator::GT)
            return val1 > val2;
        if (m_Type == Parsing::LogicalOperator::LT)
            return val1 < val2;
        if (m_Type == Parsing::LogicalOperator::LT)
            return val1 >= val2;
        if (m_Type == Parsing::LogicalOperator::LTE)
            return val1 <= val2;
        if (m_Type == Parsing::LogicalOperator::NE)
            return val1 != val2;

        std::cerr << "Erreur \n"; // il faudrait changer les erreurs pour les rendres plus explicites
        return false;
    }
};

class Comparateur_list { // verifie si deux élements respectent bien une suite de comparaison 
private:
    std::vector<Comparateur> m_Cond;

public:
    Comparateur_list(std::vector<Comparateur> cond)
        : m_Cond(cond)
    {
    }

    bool Eval(ColumnData val1, ColumnData val2)
    {
        for (Comparateur p : m_Cond) {
            if (!p.Eval(val1,val2)) {
                return false;
            }
        }
        return true;
    };
};
class Predicat { // verifie si un element respecte un prédicat
private:
    Database::Parsing::LogicalOperator m_Type;
    Database::Querying::ColumnData m_Var;

public:
    Predicat(ColumnData val, Parsing::LogicalOperator type)
        : m_Type(type)
        , m_Var(val)
    {
    }
    bool Eval(ColumnData val)
    {
        if (m_Type == Parsing::LogicalOperator::EQ)
            return val == m_Var;
        if (m_Type == Parsing::LogicalOperator::GT)
            return val > m_Var;
        if (m_Type == Parsing::LogicalOperator::LT)
            return val < m_Var;
        if (m_Type == Parsing::LogicalOperator::GTE)
            return val >= m_Var;
        if (m_Type == Parsing::LogicalOperator::LTE)
            return val <= m_Var;
        if (m_Type == Parsing::LogicalOperator::NE)
            return val != m_Var;

        std::cerr << "Erreur\n";
        return false;
    }
};
class Predicat_list { // verifie si un élement respecte bien une suite de prédication qu'on appelle Condition
private:
    std::vector<Predicat> m_Cond;

public:
    Predicat_list(std::vector<Predicat> cond)
        : m_Cond(cond)
    {
    }

    bool Eval(const ColumnData val) const
    {
        for (Predicat p : m_Cond) {
            if (!p.Eval(val)) {
                return false;
            }
        }
        return true;
    };
};
};