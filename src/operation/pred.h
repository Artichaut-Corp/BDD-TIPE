#include "../parser.h"
#include <iostream>

#include "../algebrizer_types.h"

#ifndef PRED_H

#define PRED_H

namespace Database::QueryPlanning {

using namespace Database::QueryPlanning;
class Comparateur { // permet de comparer deux elements
private:
    Database::Parsing::LogicalOperator m_Type;

public:
    Comparateur(Parsing::LogicalOperator type)
        : m_Type(type)
    {
    }

    bool Eval(ColumnData val1, ColumnData val2)
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

        std::cerr << "Erreur à l'éval du join\n"; // il faudrait changer les erreurs pour les rendres plus explicites
        return false;
    }
    Database::Parsing::LogicalOperator GetLO() { return m_Type; }
};
}

#endif // !PRED_H