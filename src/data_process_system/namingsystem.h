#ifndef NAMING_SYSTEM_H
#define NAMING_SYSTEM_H

#include <format>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_set>

namespace Database::QueryPlanning {

class TableNamesSet {
private:
    std::unordered_set<std::string> m_ListOfName;

    std::string m_NameInMemory;

    std::string m_MainName;

public:
    TableNamesSet() = default;

    explicit TableNamesSet(const std::string& main_name)
        : m_ListOfName()
        , m_NameInMemory(main_name)
        , m_MainName(m_NameInMemory)
    {
        m_ListOfName.emplace(m_NameInMemory);
        m_ListOfName.emplace(m_MainName);
    }

    bool TableEqual(const TableNamesSet& other) const noexcept
    {
        for (const auto& e : m_ListOfName) {
            for (const auto& f : other.m_ListOfName) {
                if (e == f) {
                    return true;
                }
            }
        }
        return m_MainName == other.m_MainName;
    }

    std::string GetMainName() const
    {
        return m_MainName;
    }

    std::string GetNameInMemory() const
    {
        return m_NameInMemory;
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
    return first.TableEqual(second);
}

class ColonneNamesSet {
private:
    std::string m_MainAlias;

    // nom principal unique
    std::string m_MainName;

    // pointeur nullable, ownership externe
    std::reference_wrapper<TableNamesSet> m_ParentTable;

    // tous les noms possibles (Union entre le main name, alias et pour chaque noms de la table, table.(main name ou alias de la colonne))
    std::unordered_set<std::string> m_ListOfFullName;

    // alias possibles(sans le main name)
    std::unordered_set<std::string> m_Aliases;

public:
    // Constructeur avec table
    ColonneNamesSet(std::string mainName_, std::unordered_set<std::string>* aliases, TableNamesSet& table)
        : m_ListOfFullName()
        , m_MainName(mainName_)
        , m_Aliases(*aliases)
        , m_ParentTable(table)
    {
        m_ListOfFullName.emplace(m_MainName);

        for (const auto& alias : m_Aliases) {
            m_ListOfFullName.emplace(alias);
        }

       
            for (const auto& tName : table.GetAllNames()) {
                m_ListOfFullName.emplace(std::format("{}.{}", tName, m_MainName));
                for (const auto& cName : GetAlias()) {
                    m_MainAlias = cName;
                    m_ListOfFullName.emplace(std::format("{}.{}", tName, cName));
                }
            }
        
    }

    // Constructeur sans table (colonne générique)
    ColonneNamesSet(std::string mainName, std::unordered_set<std::string> aliases)
        : m_ListOfFullName()
        , m_MainName(std::move(mainName))
        , m_Aliases(std::move(aliases))
        , m_ParentTable(*(new TableNamesSet()))

    {
        for (const auto& alias : m_Aliases) {
            m_MainAlias = alias;
            m_ListOfFullName.emplace(alias);
        }
    }
    const std::unordered_set<std::string>& GetAlias() const { return m_Aliases; }

    std::string GetMainName() const
    {
        if (m_ParentTable.get().GetMainName() != "") {
            auto tName = m_ParentTable.get().GetMainName();
            return std::format("{}.{}", tName, m_MainName);
        } else {
            return m_MainName;
        }
    }

    const std::unordered_set<std::string>& GetAllFullNames() const { return m_ListOfFullName; }

    TableNamesSet* GetTableSet() const
    {
        
            return &m_ParentTable.get();
    }

    bool HaveTableSet() const { return m_ParentTable.get().GetMainName() != ""; }

    void AddColumn(const std::string& column) { m_ListOfFullName.insert(column); }

    void FusionColumn(const ColonneNamesSet& other)
    {
        m_ListOfFullName.insert(other.m_ListOfFullName.begin(), other.m_ListOfFullName.end());
    }

    void SetTableSet(TableNamesSet* new_table)
    {
        m_ParentTable = *new_table;

        
            for (const auto& tName : m_ParentTable.get().GetAllNames()) {
                m_ListOfFullName.emplace(std::format("{}.{}", tName, m_MainName));
                for (const auto& cName : GetAlias()) {
                    m_ListOfFullName.emplace(std::format("{}.{}", tName, cName));
                }
            }
       
    }

    std::string GetMainAliasName() const
    {
        if (m_Aliases.size() != 0) {
            if (m_ParentTable.get().GetMainName()  != "") {
                auto tName = m_ParentTable.get().GetMainName();
                return std::format("{}.{}", tName, m_MainAlias);
            } else {
                return m_MainName;
            }
        } else {
            return GetMainName();
        }
    }
};

inline bool operator==(const ColonneNamesSet& first, const ColonneNamesSet& second) noexcept
{
    for (const auto& n1 : first.GetAllFullNames()) {
        if (second.GetAllFullNames().count(n1)) {
            if (first.HaveTableSet() && second.HaveTableSet() && n1.find(".") == std::string::npos) { // if the column are the same but the name test doesn't include the table, we need to be sure they have the same table name
                if (first.GetTableSet() == second.GetTableSet()) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }
    return false;
}

inline std::ostream& operator<<(std::ostream& out, const ColonneNamesSet& c)
{
    out << c.GetMainName();
    return out;
}

} // namespace

#endif // NAMING_SYSTEM_H
