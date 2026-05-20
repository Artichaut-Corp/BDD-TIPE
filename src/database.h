#include "errors.h"
#include "parser.h"
#include "repl.h"
#include "server.h"
#include "storage.h"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <replxx.h>
#include <string>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parser.hpp>
#include <vector>

#ifndef DATABASE_H

#define DATABASE_H

namespace Database {

struct DatabaseSetting {
public:
    // General Parameters
    bool m_Repl = true;

    std::string m_FileName;

    // Optimisation levels

    // Either used or not
    bool m_SelectionDescent = false;

    // See Node::Pronf function in src/algebrizer/tree.cpp in order to understand what each number do, actually defined are 0,1,3
    uint8_t m_ExecutionTreeTraversalMode = 0;

    bool m_ProjectionInsertion = 0;

    bool m_BinaryExpressionOptimization = 0;

    bool m_QueryJoinOrdering = 0;

    bool m_Benchmarking = 0;

    bool m_SizeSample = 0;

    DatabaseSetting() = default;

    DatabaseSetting(const std::string& fname, bool selectiondescent = 0, uint8_t execution_tree_traversal_mode = 0,
        bool projection_insertion = 0,
        bool binary_expression_optimization = 0,
        bool query_join_ordering = 0,
        bool benchmarking = 0,
        int SizeSample = 1000)
        : m_FileName(fname)
        , m_SelectionDescent(selectiondescent)
        , m_ExecutionTreeTraversalMode(execution_tree_traversal_mode)
        , m_ProjectionInsertion(projection_insertion)
        , m_BinaryExpressionOptimization(binary_expression_optimization)
        , m_QueryJoinOrdering(query_join_ordering)
        , m_Benchmarking(benchmarking)
        , m_SizeSample(SizeSample)
    {
    }

    DatabaseSetting(const std::string& fname, const std::string& config_fname)
        : m_FileName(fname)
    {
        try {
            auto tbl = toml::parse_file(config_fname);

            m_SelectionDescent = tbl["SelectionDescent"].value_or(0);

            m_ExecutionTreeTraversalMode = tbl["PronfMode"].value_or(0);

            m_ProjectionInsertion = tbl["InsertProj"].value_or(0);

            m_BinaryExpressionOptimization = tbl["OptimizeBinaryExpression"].value_or(0);
            m_QueryJoinOrdering = tbl["OrderingQueryJoin"].value_or(0);

            m_Benchmarking = tbl["Benchmarking"].value_or(0);
            m_Benchmarking = tbl["SizeSample"].value_or(1000);

        } catch (const toml::parse_error& err) {
            std::cerr << "Error parsing config file: " << err.description() << std::endl;
            std::cerr << "Using default values.\n";
        }
    }
};

class DatabaseEngine {

private:
    std::unique_ptr<Storing::DBTableIndex> Index;

    Storing::DBTableOrder TableOrder = {};

    Storing::File* File;

    DatabaseSetting Settings;

    // Finding default *.db database in the current directory
    // else returns an empty string
    auto FindDBFile() -> const std::string;

    // Cette méthode n'est normalement appelée qu'une fois lors de la création du
    // fichier .db. Elle initialise les tables qui contiendront les méta-données
    // sur les futures tables
    auto InitializeSystemTables(int fd) -> void;

    auto CreateTable(int fd, const std::string& name, Storing::TableInfo t) -> void;

    // Prépare et charge l'index des tables déjà présentes en mémoire
    auto FillIndex() -> void;

    auto Eval(const std::string& input) -> const std::string;

    auto PrintIndex(std::ostream& out) -> void;

    auto Cleanup() -> void
    {
        int fd = File->Fd();

        int table_count = Storing::File::GetTableCount(fd);

        auto tables_element_count = std::vector<DbInt>();

        tables_element_count.reserve(table_count);

        // Get all element_count from Index
        for (const std::string& table : TableOrder) {
            uint32_t element_count = Index->at(table).GetElementNumber();

            tables_element_count.emplace_back(element_count);
        }

        // Overwrite those values at the right place

        uint32_t offset = SCHEMA_TABLE_OFFSET + MAX_TABLE * (DB_STRING_SIZE + DB_BOOL_SIZE);

        lseek(fd, offset, SEEK_SET);

        DbInt buffer;

        for (int i = 0; i < table_count; i++) {

            buffer = tables_element_count[i];

            int bytes_written = write(fd, &buffer, DB_UINT_SIZE);
        }

        close(fd);
    }

public:
    bool m_Quit = false;

    // Options:
    // --serve address / -s address
    // --repl / -r
    // --file name / -f name
    static auto ParseArguments(int argc, char** argv) -> DatabaseSetting*;

    auto Init(DatabaseSetting* s) -> void
    {
        using namespace Database::Storing;

        Settings = *s;

        std::string db_path = "";

        bool created_file = false;

        Index = std::make_unique<Storing::DBTableIndex>();

        if (Settings.m_FileName.empty()) {

            db_path = FindDBFile();

            if (db_path == "") {
                created_file = true;

                db_path = std::format(
                    "{}/{}", std::filesystem::current_path().string(), "main.db");

                int fd = open(db_path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

                Storing::File::AddDatabaseSignature(fd);

                Storing::File::SetTableCount(fd, 0);

                // Write System Tables
                InitializeSystemTables(fd);

                // -- Country Table --
                // Text name
                // Int pop

                ColumnInfo country_name = ColumnInfo(DbElemType::DbString, false);

                ColumnInfo country_pop = ColumnInfo(DbElemType::DbUInt, false);

                auto country_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "name", country_name }, { "pop", country_pop }
                };

                auto country = TableInfo(false, 2, 0, country_columns);

                CreateTable(fd, "country", country);

                // -- City Table --
                // Text name
                // Int pop
                // Text country

                ColumnInfo city_name = ColumnInfo(DbElemType::DbString, false);

                ColumnInfo city_pop = ColumnInfo(
                    DbElemType::DbUInt, false);

                ColumnInfo city_country = ColumnInfo(
                    DbElemType::DbString, false);

                auto city_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "name", city_name }, { "pop", city_pop }, { "country", city_country }
                };

                auto city = TableInfo(false, 3, 0, city_columns);

                CreateTable(fd, "city", city);

                // -- President Table --
                // Text first_name
                // Text last_name
                // Text country
                // Int mandate_beginning

                ColumnInfo pres_first_name = ColumnInfo(DbElemType::DbString, false);

                ColumnInfo pres_last_name = ColumnInfo(DbElemType::DbString, false);

                ColumnInfo pres_mandate_beg = ColumnInfo(
                    DbElemType::DbUInt, false);

                ColumnInfo pres_country = ColumnInfo(DbElemType::DbString, false);

                auto pres_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "first_name", pres_first_name }, { "last_name", pres_last_name }, { "country", pres_country }, { "mandate_beginning", pres_mandate_beg }
                };

                auto president = TableInfo(false, 4, 0, pres_columns);

                CreateTable(fd, "president", president);

                auto page_id = ColumnInfo(DbElemType::DbUInt, false);

                auto page_ns = ColumnInfo(DbElemType::DbUInt8, false);

                auto page_title = ColumnInfo(DbElemType::DbString, false);

                auto page_revision_id = ColumnInfo(DbElemType::DbUInt, false);

                auto page_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", page_id }, { "ns", page_ns }, { "title", page_title }, { "revision_id", page_revision_id }
                };

                auto pages = TableInfo(false, 4, 0, page_columns);

                CreateTable(fd, "pages", pages);

                auto revision_id = ColumnInfo(DbElemType::DbUInt, false);

                auto revision_parent_id = ColumnInfo(DbElemType::DbUInt, false);

                auto revision_timestamp = ColumnInfo(DbElemType::DbUInt64, false);

                auto revision_contributor_id = ColumnInfo(DbElemType::DbUInt, false);

                auto revision_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", revision_id },
                    { "parent_id", revision_parent_id },
                    { "timestamp", revision_timestamp },
                    { "contributor_id", revision_contributor_id },
                };

                auto revisions = TableInfo(false, 4, 0, revision_columns);

                CreateTable(fd, "revisions", revisions);

                auto contr_id = ColumnInfo(DbElemType::DbUInt, false);

                auto contr_username = ColumnInfo(DbElemType::DbString, false);

                auto contr_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", contr_id },
                    { "username", contr_username },
                };

                auto contributors = TableInfo(false, 2, 0, contr_columns);

                CreateTable(fd, "contributors", contributors);

                auto ns_key = ColumnInfo(DbElemType::DbUInt, false);

                auto ns_name = ColumnInfo(DbElemType::DbString, false);

                auto ns_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "key", ns_key },
                    { "name", ns_name },
                };

                auto ns = TableInfo(false, 2, 0, ns_columns);

                CreateTable(fd, "namespaces", ns);

                // Clean up and close
                close(fd);
            }
        } else {
            db_path = Settings.m_FileName;
        }

        File = new Storing::File(db_path);

        if (!created_file) {
            FillIndex();
        }
        if (!s->m_Benchmarking) {
#ifdef _GLIBCXX_DEBUG_ONLY
            PrintIndex(std::cout);
#endif
        }
    }

    auto Exec(const std::string& req) -> std::string
    {
        // Should sanitize input
        std::string result;

        try {
            result = Eval(req);
        } catch (const Errors::Error& e) {
            result = e.formatErrorInfo();
        }

        return result;
    }

    auto Run() -> void
    {
        std::string input;

        // Decide if we either print the results to stdout or if the request's result needs to be handled by python
        if (Settings.m_Repl) {
            Replxx* rx = replxx_init();

            // replxx_set_completion_callback(rx, Utils::Repl::completion_callback, nullptr);
            // replxx_set_highlighter_callback(rx, Utils::Repl::highlighter_callback);

            std::string prompt = "bdd-tipe> ";

            for (;;) {
                input = replxx_input(rx, prompt.c_str());

                if (input.empty()) {
                    std::cout << "Exiting...\n";
                    m_Quit = true;

                    break;
                } else if (input.starts_with(".insert_data")) {

                    int hm = std::stoi(input.substr(13, input.length() - 13));

                    std::cout << "Insertion de " << hm << " données\n";

                    ImportAllCsv(hm);

                    std::cout << std::endl;
                } else if (input == ".print_table_layout") {
                    PrintIndex(std::cout);
                } else {

                    Utils::Repl::Print(Eval(input));
                }

                // Add to history
                replxx_history_add(rx, input.c_str());
            }

            replxx_end(rx);
        } else {

            /*
              if (Settings.m_Address == "") {
                  throw Errors::Error(Errors::ErrorType::CLIArgument, "Use of --serve / -s requires an address", 0, 0, Errors::ERROR_UNGIVEN_ARGUMENT);
              }

              const std::(string& delimiter = ":";

              auto delimiter_position = Settings.m_Address.find(delimiter);

              const std::string& address = Settings.m_Address.substr(0, delimiter_position);

              int port;

              try {
                  port = std::stoi(Settings.m_Address.substr(delimiter_position + 1, Settings.m_Address.size()));

              } catch (const std::invalid_argument& e) {

                  throw Errors::Error(Errors::ErrorType::CLIArgument, "Was not able to parse port number", 0, 0, Errors::ERROR_UNKNOWN_ARGUMENT);
              }

              Utils::SocketOStream stream
                  = Utils::Server::ConnectTcpStream(address, port);

              for (;;) {

                  input = Utils::Server::ReadStream(stream);

                  if (input == "\0") {
                      std::cout << "Exiting...\n";
                      break;
                  }

                  Utils::Server::PrintStream(stream, Eval(input));
              }

          */
        }
    }

    ~DatabaseEngine()
    {
        Cleanup();
    }

    void ImportAllCsv(int how_many);
    void InsertCsvData(int offset, int how_many);

    void ProcessCsvStream(const std::string& path, const std::string& table, const std::vector<std::string>& columns, int how_many);
    void ProcessCsvStream(const std::string& path, const std::string& table, const std::vector<std::string>& columns, int offset, int how_many);
};
}

#endif // !DATA
