#include "ast_types.h"

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, Select s)
{
    os << "Select ";
   
    return os;
};
} // namespace parsing