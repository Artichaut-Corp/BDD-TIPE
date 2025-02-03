#include "ast_types.h"

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, GroupBy s)
{
    os << "Group By";

    return os;
};

} // namespace parsing