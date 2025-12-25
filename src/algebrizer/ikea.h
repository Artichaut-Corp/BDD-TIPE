// le nom vien du faite que la classe contient plussieur Table, comme ikea "drumroll.mp3"
#include "../data_process_system/table.h"
#include <vector>
#ifndef IKEA_H

#define IKEA_H

namespace Database::QueryPlanning {

class Ikea {
private:
    std::unique_ptr<std::unordered_map<std::string, Table>> m_Catalogue;

public:
    Ikea(std::vector<Table>& lane)
    {
        m_Catalogue = std::make_unique<std::unordered_map<std::string, Table>>();

        for (int i = 0; i < lane.size(); i++) {

            m_Catalogue->insert({ lane.at(i).Get_name(), std::move(lane.at(i)) });
        }
    }

    Table GetTableByName(const std::string& name) { return std::move(m_Catalogue->at(name)); }
};
};

#endif // ! IKEA_H
