//le nom vien du faite que la classe contient plussieur Table, comme ikea "drumroll.mp3"
#include "../data_process_system/table.h"
#include <vector>
namespace Database::QueryPlanning {

class Ikea {
private:
    std::vector<std::shared_ptr<Table>> allée;
    std::map<std::string,int> catalogue;

public:
    Ikea(std::vector<std::shared_ptr<Table>> allée_)
        : allée(allée_)
    {
        std::map<std::string,int> catalogue;
        for(int i = 0;i<allée.size();i++){
            catalogue[allée[i]->Get_name()] = i;
        }
    }

    std::shared_ptr<Table> GetTableByName(std::string nom){return allée[catalogue[nom]];}

};
};