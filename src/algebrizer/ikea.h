// le nom vien du faite que la classe contient plussieur Table, comme ikea *drumroll.mp3*
#include "../data_process_system/table.h"
#include <unordered_map>
#include <vector>
#ifndef IKEA_H

#define IKEA_H

namespace Database::QueryPlanning {

class Ikea {
private:
    std::unordered_map<std::string, Table*> m_Catalogue;

public:
    Ikea(std::vector<Table>& allée_)
    {

        for (int i = 0; i < allée_.size(); i++) {

            m_Catalogue.insert({ allée_[i].Get_name()->GetMainName(), &allée_[i] });
        }
    }

    Table* GetTableByName(TableNamesSet* nom) { return m_Catalogue.at(nom->GetMainName()); }
};
};

#endif // ! IKEA_H