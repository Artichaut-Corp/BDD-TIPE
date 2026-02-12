#include "algebrizer_types.h"
#include "database.h"

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
    std::unique_ptr<ColonneNamesSet> m_ColumnName;

    DbElemType m_DataType;

    Column m_Data;

public:
    Racine(ColonneNamesSet* column_name, int fd, Storing::DBTableIndex* Index)
        : m_ColumnName(std::unique_ptr<ColonneNamesSet>(column_name))
    {
        std::string table_name = m_ColumnName->GetTableSet()->GetNameInMemory();
        std::string col_name = m_ColumnName->GetMainName().substr(m_ColumnName->GetMainName().find(".") + 1);

        std::variant<TypedColumn, Errors::Error> result = Storing::Store::DB_GetColumn(fd, Index, table_name, col_name);

        if (std::holds_alternative<Errors::Error>(result)) {
            Errors::Error e = std::get<Errors::Error>(result);

            throw e;
        }

        TypedColumn col = std::move(std::get<TypedColumn>(result));

        m_DataType = col.first;
        m_Data = std::move(col.second);
    }

    Racine(Racine&& other) noexcept = default;

    Racine& operator=(Racine&&) noexcept = default;

    Racine& operator=(const Racine&) = delete;

    Racine(const Racine& other) = delete;
    /*
      Racine(const Racine& other)
          : m_ColumnName(std::move(other.m_ColumnName.get()))
          , m_DataType(other.m_DataType)
      {

          Column* col = new Column();

          switch (other.m_DataType) {
          case DbElemType::DbNull:
              break;
          case DbElemType::DbBool:
              m_Data = std::move( std::get<std::unique_ptr<std::vector<DbBool>>>(other.m_Data));
              break;
          case DbElemType::DbInt8:
              m_Data = std::get<std::unique_ptr<std::vector<DbInt8>>>(other.m_Data);
          case DbElemType::DbUInt8:
              m_Data = std::get<std::unique_ptr<std::vector<DbUInt8>>>(other.m_Data);
          case DbElemType::DbInt16:
              m_Data = std::get<std::unique_ptr<std::vector<DbInt16>>>(other.m_Data);
          case DbElemType::DbUInt16:
              m_Data = std::get<std::unique_ptr<std::vector<DbUInt16>>>(other.m_Data);
          case DbElemType::DbInt:
              m_Data = std::get<std::unique_ptr<std::vector<DbInt>>>(other.m_Data);
          case DbElemType::DbUInt:
              m_Data = std::get<std::unique_ptr<std::vector<DbUInt>>>(other.m_Data);
          case DbElemType::DbInt64:
              m_Data = std::get<std::unique_ptr<std::vector<DbInt64>>>(other.m_Data);
          case DbElemType::DbUInt64:
              m_Data = std::get<std::unique_ptr<std::vector<DbUInt64>>>(other.m_Data);
          case DbElemType::DbFloat:
              m_Data = std::get<std::unique_ptr<std::vector<DbFloat>>>(other.m_Data);
          case DbElemType::DbFloat64:
              m_Data = std::get<std::unique_ptr<std::vector<DbFloat64>>>(other.m_Data);
          case DbElemType::DbChar:
              m_Data = std::get<std::unique_ptr<std::vector<DbChar>>>(other.m_Data);
          case DbElemType::DbString:
              m_Data = std::get<std::unique_ptr<std::vector<DbString>>>(other.m_Data);
          default:
              m_Data = Column();
          }
      }

    */

    template <typename T = DbElemType>
    inline T GetValueOfType(int i) const
    {

        auto& column = std::get<std::unique_ptr<std::vector<T>>>(m_Data);

        if (!column || i >= column->size()) {
            // Should return proper error
            throw std::out_of_range("Index hors limites");
        }

        return column->at(i);
    }

    ColumnData GetValueAt(int i) const
    {

        switch (m_DataType) {
        case DbElemType::DbNull:
            return 0;
        case DbElemType::DbBool:
            return GetValueOfType<DbBool>(i);
        case DbElemType::DbInt8:
            return GetValueOfType<DbInt8>(i);
        case DbElemType::DbUInt8:
            return GetValueOfType<DbUInt8>(i);
        case DbElemType::DbInt16:
            return GetValueOfType<DbInt16>(i);
        case DbElemType::DbUInt16:
            return GetValueOfType<DbUInt16>(i);
        case DbElemType::DbInt:
            return GetValueOfType<DbInt>(i);
        case DbElemType::DbUInt:
            return GetValueOfType<DbUInt>(i);
        case DbElemType::DbInt64:
            return GetValueOfType<DbInt64>(i);
        case DbElemType::DbUInt64:
            return GetValueOfType<DbUInt64>(i);
        case DbElemType::DbFloat:
            return GetValueOfType<DbFloat>(i);
        case DbElemType::DbFloat64:
            return GetValueOfType<DbFloat64>(i);
        case DbElemType::DbChar:
            return GetValueOfType<DbChar>(i);
        case DbElemType::DbString:
            return GetValueOfType<DbString>(i);
        default:
            return 0;
        }
    }

    int size() const
    {
        return std::visit([](auto const& vecPtr) -> int {
            return vecPtr ? vecPtr->size() : 0;
        },
            m_Data);
    }

    ColonneNamesSet& GetName()
    {
        return *m_ColumnName.get();
    }

    void AddName(const ColonneNamesSet& colname)
    {
        m_ColumnName->FusionColumn(colname);
    }
};

} // Database::QueryPlanning

#endif
