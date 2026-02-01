// le nom vien du faite que la classe contient plussieur Table, comme ikea *drumroll.mp3*
#include "data_process_system/meta-table.h"
#include <memory>
#include <unordered_map>
#include <vector>

#ifndef IKEA_H

#define IKEA_H

namespace Database::QueryPlanning {

class Ikea {
private:
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<MetaTable>>> m_Catalogue;

public:
    Ikea(std::vector<std::unique_ptr<MetaTable>>& lane)
    {
        m_Catalogue = std::make_unique<std::unordered_map<std::string, std::unique_ptr<MetaTable>>>();

        for (int i = 0; i < lane.size(); i++) {
            m_Catalogue->at(lane[i]->GetName()) = std::move(lane[i]);
        }
    }

    MetaTable* GetTableByName(const TableNamesSet& nom) { return m_Catalogue->at(nom.GetMainName()).get(); }
};
};

#endif // ! IKEA_H
