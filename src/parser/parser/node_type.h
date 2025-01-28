#ifndef NODE_TYPE_H

#define NODE_TYPE_H

namespace Compiler::Parsing {

enum class NodeType {
    Start,
    End,
    Identifier,
    FunctionDefinition,

};

enum class Type {
     Int,
     String,
};

} // namespace Compiler::Parsing

#endif // !NODE_TYPE_H



