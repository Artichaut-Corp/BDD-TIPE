#include "ast_types.h"
#include <iostream>


namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, Join s)
{	
	for (param_join e:s.param){
    switch (e) {
    case LEFT:
    	std::cout << "LEFT";
	case RIGHT:
    	std::cout << "RIGHT";
    case INNER:
    	std::cout << "INNER";
    case OUTER:

    	std::cout << "OUTER";
    case FULL:
    	std::cout << "FULL";
    };
	}
	std::cout <<"join";

    return os;
}
} // namespace parsing