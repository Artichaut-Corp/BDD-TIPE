#include "ast_types.h"
#include "node_type.h"

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, Order s)
{
    os << "Order BY ";
    if (s.BoolAsc){
		os<<" ASC ";
	}else {
	os<<" DSC ";
	}
        return os;
};
} // namespace parsing