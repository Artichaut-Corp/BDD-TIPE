#include "ast_types.h"
#include "node_type.h"

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, Having s)
{
    os << "Having";

    return os;
};
} // namespace parsing