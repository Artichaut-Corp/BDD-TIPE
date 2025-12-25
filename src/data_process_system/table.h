#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../algebrizer_types.h"
#include "colonne.h"

#ifndef TABLE_OP

#define TABLE_OP

namespace Database::QueryPlanning {

class Table {
private:
    std::unique_ptr<std::unordered_map<std::string, Colonne>> m_Map; // permet de trouver la position d'une colonne à partir de son nom

    std::unique_ptr<std::vector<std::string>> m_ColumnNames; // contient les noms de toute les colonnes présente dans la table (ces noms sont de la forme Table@colonnes) avec table étant la table originel( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci

    std::string m_Name = "";

public:
    Table() = default;

    Table(std::vector<Colonne>* data, std::string name)
        : m_Name(name)
    {
        m_Map = std::make_unique<std::unordered_map<std::string, Colonne>>();

        m_ColumnNames = std::make_unique<std::vector<std::string>>();

        for (Colonne& c : *data) {
            std::string name = c.getName();
            m_Map->insert({ name, std::move(c) });

            m_ColumnNames->push_back(name);
        }
    }

    void Selection(const Parsing::BinaryExpression::Condition pred, const std::unique_ptr<std::unordered_set<std::string>> nom_colonnes);
    void Projection(std::unique_ptr<std::vector<std::string>> ColumnToSave);

    int size()
    {
        return m_Map->size();
    }

    bool isUninitialized() const { return m_Name == ""; }

    int Columnsize() const
    {
        return m_Map->at(m_ColumnNames->at(0)).size();
    }

    ColumnData GetValue(const std::string& column_name, int pos_ind) const
    {
        return m_Map->at(column_name).getValue(pos_ind);
    }

    bool DoColumnExists(const std::string& tested_key) const
    {
        // This test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
        return !(m_Map->end() == m_Map->find(tested_key));
    }
    std::string Get_name() { return m_Name; };

    std::unique_ptr<std::unordered_map<std::string, Colonne>> GetMap()
    {
        return std::move(m_Map);
    };

    std::unique_ptr<std::vector<Colonne>> getTableCols() const
    {
        auto res = std::make_unique<std::vector<Colonne>>();

        res->reserve(m_Map->size());

        for (auto& e : *m_Map) {
            res->emplace_back(std::move(e.second));
        }

        return res;
    }
    std::unique_ptr<std::vector<std::string>> GetColumnNames() { return std::move(m_ColumnNames); }
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
