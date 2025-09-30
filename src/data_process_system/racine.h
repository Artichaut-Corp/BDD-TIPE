#include "../algebrizer_types.h"
#include "../database.h"

#include <cstddef>
#include <memory>
#include <vector>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    std::string NomColonne;
    std::variant<std::unique_ptr<std::vector<DbString>>, std::unique_ptr<std::vector<DbInt>>, std::unique_ptr<std::vector<DbInt16>>, std::unique_ptr<std::vector<DbInt8>>> data; // données immuables
public:
    Racine(std::string NomColonne_, int fd, Storing::DBTableIndex* IndexGet)
        : NomColonne(NomColonne_)
    {
        std::variant<Column, Errors::Error> ValeurRecuper = Storing::Store::GetDBColumn(fd, IndexGet, NomColonne_.substr(0, NomColonne_.find(".")), NomColonne_.substr( NomColonne_.find(".")+1));
        if (std::holds_alternative<Errors::Error>(ValeurRecuper)) {
            Errors::Error e = std::get<Errors::Error>(ValeurRecuper);
            throw e;
        }

        auto column_data = std::get<Column>(std::move(ValeurRecuper));

        if (std::holds_alternative<std::unique_ptr<std::vector<DbString>>>(column_data)) {
            data = std::get<std::unique_ptr<std::vector<DbString>>>(std::move(column_data));

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt>>>(column_data)) {
            data = std::get<std::unique_ptr<std::vector<DbInt>>>(std::move(column_data));

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt16>>>(column_data)) {
            data = std::get<std::unique_ptr<std::vector<DbInt16>>>(std::move(column_data));

        } else {
            data = std::get<std::unique_ptr<std::vector<DbInt8>>>(std::move(column_data));
        }
    }
    Racine(const Racine& other)
        : NomColonne(other.NomColonne)
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
