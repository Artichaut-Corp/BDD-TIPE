#include "../algebrizer_types.h"
#include "../database.h"

#include <memory>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {

private:
    std::string m_ColumnName;
    Column data; // données immuables

public:
    Racine(const std::string& column_name, int fd, Storing::DBTableIndex* Index)
        : m_ColumnName(column_name)
    {
        std::variant<Column, Errors::Error> col = Storing::Store::DB_GetColumn(fd, Index, column_name.substr(0, column_name.find(".")), column_name.substr(column_name.find(".") + 1));

        if (std::holds_alternative<Errors::Error>(col)) {
            Errors::Error e = std::get<Errors::Error>(col);
            throw e;
        }

        data = std::move(std::get<Column>(col));
    }

    Racine(const Racine& other)
        : m_ColumnName(other.m_ColumnName)
    {
        data = std::visit([](auto const& vecPtr) -> decltype(data) {
            using VecType = std::decay_t<decltype(*vecPtr)>;
            if (!vecPtr) {
                // retourne un unique_ptr vide, mais du bon type
                return std::unique_ptr<VecType> {};
            }
            // copie du vecteur sous-jacent
            return std::make_unique<VecType>(*vecPtr);
        },
            other.data);
    }

    ColumnData getValue(int i) const
    {
        return std::visit([i](auto const& vecPtr) -> ColumnData {
            if (!vecPtr || i >= vecPtr->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*vecPtr)[i]; // renvoie soit un DbString, soit un DbInt
        },
            data);
    }

    int size() const
    {
        return std::visit([](auto const& vecPtr) -> int {
            return vecPtr ? vecPtr->size() : 0;
        },
            data);
    }
};

} // Database::QueryPlanning

#endif
