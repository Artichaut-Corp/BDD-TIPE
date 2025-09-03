#include <vector>

#ifndef OP_H

#define OP_H

namespace Database::QueryPlanning {

class Agregation {
    static std::vector<int>& Sum();
    static std::vector<int>& Average();
    static std::vector<int>& Count();
    static std::vector<int>& Min();
    static std::vector<int>& Max();
};

} // Database::QueryPlanning

#endif // !OP_H OP_H
