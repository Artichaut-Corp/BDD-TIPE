// le nom vien du faite que la classe contient plussieur Table, comme ikea "drumroll.mp3"
#include "../data_process_system/table.h"
#include <vector>
#ifndef IKEA_H

#define IKEA_H


namespace Database::QueryPlanning {

class Ikea {
private:
    std::map<std::string, Table*> catalogue;

public:
    Ikea(std::vector<Table>& allée_)
    {

        for (int i = 0; i < allée_.size(); i++) {

            catalogue.insert({ allée_[i].Get_name(), &allée_[i] });
        }
    }

    Table* GetTableByName(std::string nom) { return catalogue.at(nom); }
};
};

#endif // ! IKEA_H