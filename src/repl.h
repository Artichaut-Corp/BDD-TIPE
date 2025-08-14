#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "errors.h"
#include "parser.h"
#include "storage.h"

#ifndef REPL_H

#define REPL_H

namespace Database::Utils {

class Repl {
    // Get input and lex
    static std::string Read()
    {
        std::string buf;
        getline(std::cin, buf);

        const std::string& ret = (buf == "\r\n" || buf == "\n") ? "\0" : buf;

        return ret;
    }

    static std::string Eval(const std::string& input, const Storing::File& f)
    {

        std::string output = "";

        Parsing::Parser* parser = new Parsing::Parser(input);

        std::variant<Parsing::Statement, Errors::Error> n = parser->Parse();

        if (std::holds_alternative<Errors::Error>(n)) {
            Errors::Error e = std::get<Errors::Error>(n);
            e.printAllInfo();

            return "\0";
        }

        auto stmt = std::get<Parsing::Statement>(n);

        // Traiter chaque Statement
        if (std::holds_alternative<Parsing::SelectStmt*>(stmt)) {
            auto select = std::get<Parsing::SelectStmt*>(stmt);

            auto fields = select->getFields()->getField();

            if (fields.size() != 1) {
                throw std::runtime_error("Erreur provisoire, pour l'instant une colonne à la fois pour tester.");
            }

            auto column = std::holds_alternative<Parsing::SelectField>(fields[0]) ? std::get<Parsing::SelectField>(fields[0]) : throw std::runtime_error("Erreur provisoire, on ne teste pas encore les fonctions d'aggrégation.");

            auto read_result = Storing::Store::GetDBColumn(f.Fd(), select->getTable()->getTableName(), column.m_Field.value().getColumnName());

            if (std::holds_alternative<Errors::Error>(read_result)) {
                Errors::Error e = std::get<Errors::Error>(read_result);

                e.printAllInfo();

                return "\0";
            }

            auto column_data = std::get<Column>(std::move(read_result));

            // Probablement une fonction qui affichera un joli tableau du résultat
            std::ostringstream oss;

            if (std::holds_alternative<std::unique_ptr<std::vector<DbString>>>(column_data)) {

                auto result = std::get<std::unique_ptr<std::vector<DbString>>>(std::move(column_data));

                for (size_t i = 0; i < result->size(); i++) {
                    oss << Convert::DbStringToString(result->at(i));

                    if (i < result->size() - 1) {
                        oss << ", ";
                    }
                }
            } else {
                auto result = std::get<std::unique_ptr<std::vector<DbInt>>>(std::move(column_data));

                for (size_t i = 0; i < result->size(); i++) {
                    oss << result->at(i);

                    if (i < result->size() - 1) {
                        oss << ", ";
                    }
                }
            }

            output = oss.str();

            delete select;
        } else if (std::holds_alternative<Parsing::UpdateStmt*>(stmt)) {
            auto update = std::get<Parsing::UpdateStmt*>(stmt);

            delete update;

            output = "UPDATE";
        } else if (std::holds_alternative<Parsing::InsertStmt*>(stmt)) {
            auto insert = std::get<Parsing::InsertStmt*>(stmt);

            std::string name = insert->getTable()->getTableName();

            // On va avoir besoin de sombres techniques pour savoir quelle class enployer et remplir des info recup
            if (insert->isDefault()) {
                std::cout << "TODO: insert stmt using default values.\n";
            } else if (insert->getOrder().has_value()) {

                std::unordered_map<std::string, ColumnData>* data = Storing::Record::GetMapFromData(insert->getData()->get(), insert->getOrder()->get());

                auto err = Storing::Store::SetData(f.Fd(), name, *data);

                delete data;

                if (err.has_value()) {
                    err->printAllInfo();

                    return "\0";
                }

            } else {
                // Use a default order to fill column
                // We will worry about it later

                std::cout << "TODO: insert stmt using default order.\n";
            }

            delete insert;

            output = std::format("Inserted one record into table {}.", name);

        } else if (std::holds_alternative<Parsing::Transaction*>(stmt)) {
            auto transaction = std::get<Parsing::Transaction*>(stmt);

            std::string name = transaction->getTable()->getTableName();

            auto col_order = transaction->getOrder().get();

            auto col_data = transaction->getData().get();

            size_t col_number = col_order->size();

            size_t record_number = col_data->size();

            std::unordered_map<std::string, ColumnData>* data;
            for (size_t i = 0; i < record_number / col_number; i++) {

                data = Storing::Record::GetMapFromData(
                    std::span(col_data->begin() + 2 * i, col_data->begin() + (2 * i + col_number)),
                    col_order);

                auto err = Storing::Store::SetData(f.Fd(), name, *data);

                if (err.has_value()) {
                    err->printAllInfo();

                    return "\0";
                }
            }

            delete data;

            delete transaction;

            output = std::format("Inserted {} elements into table {}.", record_number / col_number, name);
        } else if (std::holds_alternative<Parsing::DeleteStmt*>(stmt)) {
            // delete est un mot réservé en C++
            auto del = std::get<Parsing::DeleteStmt*>(stmt);

            delete del;

            output = "DELETE";
        } else {

            Storing::File::Cleanup(f.Fd());

            throw std::runtime_error("Unrecognized Statement");
        }

        delete parser;

        return output;
    }

    // Pourrait prendre un type Result qui serait formatté et affiché ici
    static void Print(const std::string& result)
    {
        std::cout << result << std::endl;
    }

public:
    static void Run(const Storing::File& f)
    {
        for (;;) {
            std::cout << "SQL>>" << std::endl;

            const std::string& input = Read();

            if (input == "\0") {
                std::cout << "Exiting...\n";

                Storing::File::Cleanup(f.Fd());

                return;
            }

            Print(Eval(input, f));
        }
    }
};

}

#endif
