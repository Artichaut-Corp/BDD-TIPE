#include "../algebrizer_types.h"
#include "../database.h"
#include "namingsystem.h"

#include <memory>
#include <variant>
#include <vector>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    std::shared_ptr<ColonneNamesSet> m_NomColonne;
    std::variant<std::shared_ptr<std::vector<DbString>>, std::shared_ptr<std::vector<DbInt>>, std::shared_ptr<std::vector<DbInt16>>, std::shared_ptr<std::vector<DbInt8>>> m_data; // données immuables
public:
    Racine(std::shared_ptr<ColonneNamesSet> NomColonne_, int fd, Storing::DBTableIndex* IndexGet)
        : m_NomColonne(NomColonne_)
    {
        std::variant<Column, Errors::Error> ValeurRecuper = Storing::Store::GetDBColumn(fd, IndexGet, NomColonne_->GetTableSet()->GetNameInMemory(), NomColonne_->GetMainName().substr(NomColonne_->GetMainName().find(".") + 1));
        if (std::holds_alternative<Errors::Error>(ValeurRecuper)) {
            Errors::Error e = std::get<Errors::Error>(ValeurRecuper);
            throw e;
        }

        auto column_data = std::get<Column>(std::move(ValeurRecuper));

        if (std::holds_alternative<std::unique_ptr<std::vector<DbString>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbString>>>(std::move(column_data));
            m_data = std::move(temp);

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt>>>(std::move(column_data));
            m_data = std::move(temp);

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt16>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt16>>>(std::move(column_data));
            m_data = std::move(temp);

        } else {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt8>>>(std::move(column_data));
            m_data = std::move(temp);
        }
    }
    Racine(const Racine& other)
        : m_NomColonne(other.m_NomColonne)
    {
        m_data = other.m_data;
    }

    ColumnData get_value_dans_ptr(int i) const
    {
        if (std::holds_alternative<std::shared_ptr<std::vector<DbString>>>(m_data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbString>>>(m_data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }else if (std::holds_alternative<std::shared_ptr<std::vector<DbInt>>>(m_data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt>>>(m_data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }else if (std::holds_alternative<std::shared_ptr<std::vector<DbInt16>>>(m_data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt16>>>(m_data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }
        else {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt8>>>(m_data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }
    }

    int size() const
    {
        return std::visit([](auto const& vecPtr) -> int {
            return vecPtr ? vecPtr->size() : 0;
        },
            m_data);
    }
    std::shared_ptr<ColonneNamesSet> get_name(){
        return m_NomColonne;
    }

    void addname(std::shared_ptr<ColonneNamesSet> colname){
        m_NomColonne->FusionColumn(colname);
    }
};

} // Database::QueryPlanning

#endif
