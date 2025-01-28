#include "ast_node.h"
#include <ostream>

namespace Compiler::Parsing {

std::ostream& operator<<(std::ostream& os, AstNode& node) { 
  os << "NODE";

  return os;
}

} // namespace Compiler::Parsing
