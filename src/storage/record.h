#include "../errors.h"
#include "../parser.h"
#include "table.h"
#include "types.h"

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef RECORD_H

#define RECORD_H

namespace Database::Storing {

class Record {

public:
    template <typename T>
    static std::unique_ptr<std::vector<T>>
    GetColumn(int fd, const ColumnInfo& info, uint16_t element_number)
    {
        // Etapes:
        // - Aller Chercher les informations de la table (peut être les garder)
        // - Lire à partir de l'offset n tous les n + |e|
        // - Remplir et retourner un vecteur

        auto result = std::make_unique<std::vector<T>>();

        uint32_t offset = info.GetOffset();

        uint8_t element_size = info.GetElementSize();

        result->reserve(element_number);

        lseek(fd, offset, SEEK_SET);

        T buffer;

        for (int i = 0; i < element_number; i++) {
            int bytes_read = read(fd, &buffer, element_size);

            offset += bytes_read;

            if (bytes_read != element_size) {
                throw std::runtime_error("failed to read");
            }

            result->emplace_back(buffer);
        }

        return result;
    }

    // Supposant que la table existe;
    // - Aller Chercher les informations de la table (peut être les garder)
    // - Ecrire à partir de l'offset + |col| à voir les pb de dépassement de
    // capacité qui meneraient éventuellement à déplacer l'entierté / une partie
    // / rien des données
    template <typename T>
    static void Write(int fd, TableInfo* info, const T& record)
    {

        const auto data = record->Map();

        int ret = 0;

        for (auto iter = info->m_Columns.begin(); iter != info->m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd, iter->second.GetOffset() + e_size * info->GetElementNumber(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        info->IncrMaxRecord();
    }

    // Supposant que la table existe;
    // - Aller Chercher les informations de la table (peut être les garder)
    // - Ecrire à partir de l'offset + |col| à voir les pb de dépassement de
    // capacité qui meneraient éventuellement à déplacer l'entierté / une partie
    // / rien des données
    template <typename T>
    static void Write(int fd, TableInfo* info, const T& record,
        const std::string& name)
    {

        auto data = record->Map(name);

        int ret = 0;

        for (auto iter = info->m_Columns.begin(); iter != info->m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd, iter->second.GetOffset() + e_size * info->GetElementNumber(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        info->IncrMaxRecord();
    }

    // Using directly a map to write data
    // Does not ensure that the TableInfo provided corresponds to the data
    static void Write(int fd, TableInfo* info, const std::unordered_map<std::string, ColumnData>& data)
    {

        int ret = 0;

        for (auto iter = info->m_Columns.begin(); iter != info->m_Columns.end();
            ++iter) {

            uint8_t e_size = iter->second.GetElementSize();

            // Ptêtre bound check quand même
            lseek(fd, iter->second.GetOffset() + e_size * info->GetElementNumber(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        info->IncrMaxRecord();
    }

    static std::unordered_map<std::string, ColumnData>* GetMapFromData(std::vector<Parsing::LitteralValue<std::string>>* column_data, std::vector<Parsing::ColumnName>* column_order)
    {

        auto res = new std::unordered_map<std::string, ColumnData>();

        // Assuming the vectors have the same length
        // I will certainly need some types to cast in
        //
        // TODO: Not functionnal till types are not casted rigth
        for (int i = 0; i < column_order->size(); i++) {

            if (column_data->at(i).getColumnType() == Parsing::ColumnType::INTEGER_C) {
                res->insert(
                    { column_order->at(i).getColumnName(), static_cast<DbInt>(std::stoi(column_data->at(i).getData())) });
            } else {

                res->insert(
                    { column_order->at(i).getColumnName(), Convert::StringToDbString(column_data->at(i).getData()) });
            }
        }

        return res;
    }

    static std::unordered_map<std::string, ColumnData>* GetMapFromData(std::span<Parsing::LitteralValue<std::string>> column_data, std::vector<Parsing::ColumnName>* column_order)
    {

        auto res = new std::unordered_map<std::string, ColumnData>();

        // Assuming the vectors have the same length
        // I will certainly need some types to cast in
        //
        // TODO: Not functionnal till types are not casted rigth
        for (int i = 0; i < column_order->size(); i++) {

            if (column_data[i].getColumnType() == Parsing::ColumnType::INTEGER_C) {
                res->insert(
                    { column_order->at(i).getColumnName(), static_cast<DbInt>(std::stoi(column_data[i].getData())) });
            } else {

                res->insert(
                    { column_order->at(i).getColumnName(), Convert::StringToDbString(column_data[i].getData()) });
            }
        }

        return res;
    }
};

}
#endif
