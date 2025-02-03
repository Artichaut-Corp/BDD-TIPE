#include "ast_types.h"
#include "node_type.h"

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os,const Where& s)
{
    os << "Where";

    return os;
};

} // namespace parsing