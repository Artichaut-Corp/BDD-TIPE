#ifndef OP_H
#define OP_H

#include <vector>

namespace Compiler::Quering {

class Agregation {
    static std::vector<int>& Sum();
    static std::vector<int>& Average();
    static std::vector<int>& Count();
    static std::vector<int>& Min();
    static std::vector<int>& Max();
};
};
#endif