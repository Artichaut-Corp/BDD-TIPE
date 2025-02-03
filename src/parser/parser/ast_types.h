#ifndef AST_H

#define AST_H

#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

namespace Compiler::Parsing {

class Expression { };

class Statement { };

enum param_join {
    LEFT,
    RIGHT,
    INNER,
    OUTER,
    FULL,
};
enum param_binary_expr {
    AND,
    OR,
    GEQ,
    LEQ,
    GT,
    LT,
    NEQ,
    EQ
};

// TBD
class From {

private:
public:
    From()
    {
    }
};
class Select {

private:
public:
    Select()
    {
    }
};
class Where {

private:
public:
    Where()
    {
    }
};
class GroupBy {

private:
public:
    GroupBy()
    {
    }
};
class Having {

private:
public:
    Having()
    {
    }
};
class Order {

private:
public:
    bool BoolAsc;
    Order(bool asc)
        : BoolAsc(asc)
    {
    }
};
class Join {

private:
    std::vector<param_join> param;

public:
    Join(std::vector<param_join> param_join)
        : param(param_join)
    {
    }
};
class BinaryExpression {

private:
    std::vector<param_binary_expr> param;
public:
    BinaryExpression(std::vector<param_binary_expr> param_binary)
        : param(param_binary)
    {
    }
};
class Limit {

private:
public:
    Limit()
    {
    }
};
class Like {

private:
public:
    Like()
    {
    }
};
class Set {

private:
public:
    Set()
    {
    }
};
class As {

private:
public:
    As()
    {
    }
};
class Delete {

private:
public:
    Delete()
    {
    }
};
class Create {

private:
public:
    Create()
    {
    }
};
class Table {

private:
public:
    Table()
    {
    }
};
class Drop {

private:
public:
    Drop()
    {
    }
};
class Union {

private:
public:
    Union()
    {
    }
};
// class Case;
} // namespace Compiler::Parsing

#endif // !AST_H
