#include "../errors.h"
#include "../parser.h"
#include "b+tree.h"
#include "table.h"
#include "types.h"

#include <cstdint>
#include <memory>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef RECORD_H

#define RECORD_H

namespace Database::Storing {

class Record {

public:
    template <typename T>
    static std::unique_ptr<std::vector<std::pair<DbKey, T>>> GetRange(int fd, const ColumnInfo& info, const Parsing::BinaryExpression& predicate);

    template <typename T>
    static std::unique_ptr<std::vector<T>> GetColumn(int fd, const ColumnInfo& info, DbKey element_number)
    {
        // Etapes:
        // - Aller Chercher les informations de la table (peut être les garder)
        // - Lire à partir de l'offset n tous les n + |e|
        // - Remplir et retourner un vecteur

        std::vector<T> result = std::make_unique<std::vector<T>>();

        uint32_t offset = info.GetOffset();

        uint8_t element_size = info.GetElementSize();

        result->reserve(element_number);

        lseek(fd, offset, SEEK_SET);

        T buffer;

        int bytes_read = read(fd, &buffer, element_size * element_number);

        if (bytes_read != element_size * element_number) {
            throw std::runtime_error("failed to read");
        }

        result->data() = buffer;

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
            lseek64(fd, iter->second.GetOffset() + e_size * info->GetElementNumber(),
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
            lseek64(fd, iter->second.GetOffset() + e_size * info->GetElementNumber(),
                SEEK_SET);

            ret = write(fd, &data.at(iter->first), e_size);
        }

        info->IncrMaxRecord();
    }

    static void WriteIndex(int fd, TableInfo* info, const std::unordered_map<std::string, ColumnData>& data)
    {
        for (auto iter = info->m_Columns.begin(); iter != info->m_Columns.end();
             ++iter) {

            if (iter->second.IsSorted()) {
                DbInt64 offset = iter->second.GetIndexOffset();

                // const DbInt8 e_size = iter->second.GetElementSize();

                BPlusTree<TREE_ORDER, e_size>::Node root = BPlusTree<TREE_ORDER>::FindRoot(fd, offset);

                BPlusTree<TREE_ORDER>::Insert(fd, root, data.at(iter->first));
            }
        }
    }

    // Utility function mapping key and values from vectors into a map
    static std::unordered_map<std::string, ColumnData>* GetMapFromData(std::vector<Parsing::LitteralValue<std::string>>* column_data, std::vector<Parsing::ColumnName>* column_order)
    {

        auto res = new std::unordered_map<std::string, ColumnData>();

        // Assuming the vectors have the same length
        // I will certainly need some types to cast in
        //
        // Solution not optimal -> would not make any difference between signed and unsigned ints
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
