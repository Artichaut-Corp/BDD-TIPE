#ifndef COLUMNNAME_H
#define COLUMNNAME_H

#include <cmath>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_set>

namespace Database::QueryPlanning {

class TableNamesSet {
private:
    std::unordered_set<std::string> m_ListOfName;
    std::string NameInMemory;
    std::string MainName;

public:
    explicit TableNamesSet(std::string mainName)
        : m_ListOfName()
        , NameInMemory(std::move(mainName))
        , MainName(NameInMemory)
    {
        m_ListOfName.emplace(NameInMemory);
        m_ListOfName.emplace(MainName);
    }

    bool TableEqual(const TableNamesSet* other) const noexcept
    {
        for (const auto& e : m_ListOfName) {
            for (const auto& f : other->m_ListOfName) {
                if (e == f) {
                    return true;
                }
            }
        }
        return MainName == other->MainName;
    }

    std::string GetMainName() const
    {
        return MainName;
    }

    const std::string& GetNameInMemory() const
    {
        return NameInMemory;
    }

    void AddAlias(const std::string& table)
    {
        m_ListOfName.insert(table);
    }

    // Retourne une référence const pour éviter l'exposition directe du pointeur
    const std::unordered_set<std::string>& GetAllNames() const { return m_ListOfName; }
};

inline bool operator==(const TableNamesSet& first, const TableNamesSet& second)
{
    return first.TableEqual(&second);
}

class ColonneNamesSet {
private:
    std::unordered_set<std::string> m_ListOfFullName; // tous les noms possibles (Union entre le main name, alias et pour chaque noms de la table, table.(main name ou alias de la colonne))
    std::string MainName; // nom principal unique
    std::unordered_set<std::string> Aliases; // alias possibles(sans le main name)
    TableNamesSet* Table; // pointeur nullable, ownership externe
    std::string MainAlias;

public:
    // Constructeur avec table
    ColonneNamesSet(std::string mainName_, std::unordered_set<std::string> aliases, TableNamesSet* table)
        : m_ListOfFullName()
        , MainName(std::move(mainName_))
        , Aliases(std::move(aliases))
        , Table(table)
    {
        m_ListOfFullName.emplace(MainName);
        for (const auto& alias : Aliases) {
            m_ListOfFullName.emplace(alias);
        }
        if (Table) {
            for (const auto& tName : Table->GetAllNames()) {
                m_ListOfFullName.emplace(std::format("{}.{}", tName, MainName));
                for (const auto& cName : GetAlias()) {
                    MainAlias = cName;
                    m_ListOfFullName.emplace(std::format("{}.{}", tName, cName));
                }
            }
        }
    }

    // Constructeur sans table (colonne générique)
    ColonneNamesSet(std::string mainName, std::unordered_set<std::string> aliases)
        : m_ListOfFullName()
        , MainName(std::move(mainName))
        , Aliases(std::move(aliases))
        , Table(nullptr)

    {
        for (const auto& alias : Aliases) {
            MainAlias = alias;
            m_ListOfFullName.emplace(alias);
        }
    }
    const std::unordered_set<std::string>& GetAlias() const { return Aliases; }
    
    std::string GetMainName() const
    {
        if (Table != nullptr) {
            auto tName = Table->GetMainName();
            return std::format("{}.{}", tName, MainName);
        } else {
            return MainName;
        }
    }

    const std::unordered_set<std::string>& GetAllFullNames() const { return m_ListOfFullName; }

    TableNamesSet* GetTableSet() const { return Table; }
    bool HaveTableSet() const { return Table != nullptr; }

    void AddColumn(const std::string& column) { m_ListOfFullName.insert(column); }

    void FusionColumn(const ColonneNamesSet* other)
    {
        m_ListOfFullName.insert(other->m_ListOfFullName.begin(), other->m_ListOfFullName.end());
    }

    void SetTableSet(TableNamesSet* newTable)
    {
        Table = newTable;
        if (Table) {
            for (const auto& tName : Table->GetAllNames()) {
                m_ListOfFullName.emplace(std::format("{}.{}", tName, MainName));
                for (const auto& cName : GetAlias()) {
                    m_ListOfFullName.emplace(std::format("{}.{}", tName, cName));
                }
            }
        }
    }
    std::string GetMainAliasName() const
    {
        if (Aliases.size() != 0) {
            if (Table != nullptr) {
                auto tName = Table->GetMainName();
                return std::format("{}.{}", tName, MainAlias);
            } else {
                return MainName;
            }
        }else {
            return GetMainName();
        }
    }
};

inline bool operator==(const ColonneNamesSet& first, const ColonneNamesSet& second) noexcept
{
    for (const auto& n1 : first.GetAllFullNames()) {
        if (second.GetAllFullNames().count(n1))
            return true;
    }
    return false;
}

inline std::ostream& operator<<(std::ostream& out, const ColonneNamesSet& c)
{
    out << c.GetMainName();
    return out;
}

} // namespace

#endif // COLUMNNAME_H
