#include "table.h"
#include "types.h"

#include <vector>

#ifndef RECORD_H

#define RECORD_H

namespace Database::Storing {

using DBTableIndex = std::unordered_map<std::string, TableInfo>;

static DBTableIndex Index = {};

class Record {

    static ColumnInfo LocateColumn(std::string table_name,
        std::string column_name)
    {
        // - Aller chercher la table système via le header qui garde sa position
        // - Parcourir jusqu'à trouver la bonne table puis la bonne colonne
        // - Remplir la structure et la retourner
    }

public:
    static Column GetColumn(int fd,
        ColumnInfo& info)
    {
        // Etapes:
        // - Aller Chercher les informations de la table (peut être les garder)
        // - Lire à partir de l'offset n tous les n + |e|
        // - Remplir et retourner un vecteur

        auto result = std::make_unique<std::vector<ColumnData>>();

        const uint64_t offset = info.GetOffset();
        const int element_count = info.GetCurrentMax();

        result->reserve(element_count);

        lseek(fd, offset, SEEK_SET);

        read(fd, result->data(), info.GetElementSize() * element_count);

        return result;
    }

    // Supposant que la table existe;
    // - Aller Chercher les informations de la table (peut être les garder)
    // - Ecrire à partir de l'offset + |col| à voir les pb de dépassement de
    // capacité qui meneraient éventuellement à déplacer l'entierté / une partie
    // / rien des données
    template <typename T>
    static uint32_t Write(int fd, TableInfo& info, T* record)
    {

        const auto data = record->Map();

        int ret = 0;

        for (auto iter = info.m_Columns.begin(); iter != info.m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd,
                iter->second.GetOffset() + e_size * iter->second.GetCurrentMax(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }
        return info.GetColumnsFirstOffset();
    }

    // Supposant que la table existe;
    // - Aller Chercher les informations de la table (peut être les garder)
    // - Ecrire à partir de l'offset + |col| à voir les pb de dépassement de
    // capacité qui meneraient éventuellement à déplacer l'entierté / une partie
    // / rien des données
    template <typename T>
    static uint32_t Write(int fd, TableInfo& info, T* record,
        const std::string& name)
    {

        auto data = record->Map(name);

        int ret = 0;

        for (auto iter = info.m_Columns.begin(); iter != info.m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd,
                iter->second.GetOffset() + e_size * iter->second.GetCurrentMax(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        return info.GetColumnsFirstOffset();
    }

    static uint32_t Write(int fd, TableInfo& info, TableInfo* record,
        const std::string& name,
        const std::vector<DbInt> offsets)
    {

        auto data = record->Map(name, offsets);

        int ret = 0;

        for (auto iter = info.m_Columns.begin(); iter != info.m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd,
                iter->second.GetOffset() + e_size * iter->second.GetCurrentMax(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        return info.GetColumnsFirstOffset();
    }
};


}
#endif
