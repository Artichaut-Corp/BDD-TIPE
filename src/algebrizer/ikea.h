// le nom vien du faite que la classe contient plussieur Table, comme ikea *drumroll.mp3*
#include "../data_process_system/meta-table.h"
#include <unordered_map>
#include <vector>
#ifndef IKEA_H

#define IKEA_H

namespace Database::QueryPlanning {

class Ikea {
private:
    std::unordered_map<std::string, std::shared_ptr<MetaTable>> m_Catalogue;

public:
    Ikea(std::vector<MetaTable>& allée_)
    {

        for (int i = 0; i < allée_.size(); i++) {
            m_Catalogue[allée_[i].Get_name()] = std::make_shared<MetaTable>(allée_[i]);
        }
    }

    std::shared_ptr<MetaTable> GetTableByName(std::shared_ptr<TableNamesSet> nom) { return m_Catalogue.at(nom->GetMainName()); }
};
};

#endif // ! IKEA_H