#include "ast_types.h"
#include <iostream>

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, 	BinaryExpression s)
{
    for (param_binary_expr e : s.param) {
        switch (e) {
        case AND:
            std::cout << " AND ";
        case OR:
        	std::cout<< " OR ";
        case LEQ:
            std::cout << " <= ";
        case GEQ:
            std::cout << " >= ";
        case EQ:
            std::cout << " == ";
		case LT:
                    std::cout << " < ";
                case GT:
                    std::cout << " > ";
		case NEQ:
                    std::cout << " != ";
        };
    }
    std::cout << "join";

	return os;
}

} // namespace parsing