#include "parser.h"

#include "parser/ast_types.h"
#include <cstdio>

namespace Compiler::Parsing {

Parser::Parser(Lexing::TokenType file_start)
    : tree(Tree(file_start))
{
}

void Parser::parse()
{
    if (t.IsEmpty()) {
        return;
    }

    bool VerifPunct = false;
    switch (t.peek().token ) {
        case SELECT_T :
            tree.AddNode(tree.GetRoot(), Select)
        case PUNCT_T :
            switch (PUNCT_T){
                case ".":

                case ";":
                    VerifPunct = true;
                case ",":
            }
        Case
    default:
        break;
    }
    if (!VerifPunct){
        printf("pas de ; à la fin de la requête") ;
        return;
    }
}

} // namespace Compiler::Parsing
