#include "../algebrizer_types.h"
#include "../database.h"

#include <memory>
#include <variant>
#include <vector>

#ifndef RACINE_H

#define RACINE_H
namespace Database::QueryPlanning {

// Racine : contient des pointeurs vers les données brutes (immutable)
class Racine {
private:
    ColonneNamesSet* NomColonne;
    std::variant<std::shared_ptr<std::vector<DbString>>, std::shared_ptr<std::vector<DbInt>>, std::shared_ptr<std::vector<DbInt16>>, std::shared_ptr<std::vector<DbInt8>>> data; // données immuables
public:
    Racine(ColonneNamesSet* NomColonne_, int fd, Storing::DBTableIndex* IndexGet)
        : NomColonne(NomColonne_)
    {
        std::variant<Column, Errors::Error> ValeurRecuper = Storing::Store::GetDBColumn(fd, IndexGet, NomColonne_->GetTableSet()->GetNameInMemory(), NomColonne_->GetMainName().substr(NomColonne_->GetMainName().find(".") + 1));
        if (std::holds_alternative<Errors::Error>(ValeurRecuper)) {
            Errors::Error e = std::get<Errors::Error>(ValeurRecuper);
            throw e;
        }

        auto column_data = std::get<Column>(std::move(ValeurRecuper));

        if (std::holds_alternative<std::unique_ptr<std::vector<DbString>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbString>>>(std::move(column_data));
            data = std::move(temp);

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt>>>(std::move(column_data));
            data = std::move(temp);

        } else if (std::holds_alternative<std::unique_ptr<std::vector<DbInt16>>>(column_data)) {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt16>>>(std::move(column_data));
            data = std::move(temp);

        } else {
            auto temp = std::get<std::unique_ptr<std::vector<DbInt8>>>(std::move(column_data));
            data = std::move(temp);
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
        if (std::holds_alternative<std::shared_ptr<std::vector<DbString>>>(data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbString>>>(data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }else if (std::holds_alternative<std::shared_ptr<std::vector<DbInt>>>(data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt>>>(data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }else if (std::holds_alternative<std::shared_ptr<std::vector<DbInt16>>>(data)) {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt16>>>(data);
            if (!temp || i >= temp->size()) {
                throw std::out_of_range("Index hors limites");
            }
            return (*temp)[i];
        }
        else {
            auto temp = std::get<std::shared_ptr<std::vector<DbInt8>>>(data);
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
            data);
    }
};

} // Database::QueryPlanning

#endif
