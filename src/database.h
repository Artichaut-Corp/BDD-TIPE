#include "errors.h"
#include "parser.h"
#include "repl.h"
#include "server.h"
#include "storage.h"

#include <filesystem>
#include <format>
#include <memory>
#include <ostream>
#include <replxx.h>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef DATABASE_H

#define DATABASE_H

namespace Database {

class DatabaseEngine {

private:
    class DatabaseSetting {
    public:
        bool m_Repl = true;
        std::string m_Address;
        std::string m_FileName;

        DatabaseSetting() = default;
    };

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

            int bytes_written = write(fd, &buffer, DB_INT_SIZE);
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

                ColumnInfo country_name = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

                ColumnInfo country_pop = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
                    DB_INT_SIZE, false);

                auto country_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "name", country_name }, { "pop", country_pop }
                };

                auto country = TableInfo(false, 2, 0, country_columns);

                CreateTable(fd, "country", country);

                // -- City Table --
                // Text name
                // Int pop
                // Text country

                ColumnInfo city_name = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

                ColumnInfo city_pop = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
                    DB_INT_SIZE, false);

                ColumnInfo city_country = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE),
                    DB_STRING_SIZE, false);

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

                ColumnInfo pres_first_name = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

                ColumnInfo pres_last_name = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

                ColumnInfo pres_mandate_beg = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
                    DB_INT_SIZE, false);

                ColumnInfo pres_country = ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

                auto pres_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "first_name", pres_first_name }, { "last_name", pres_last_name }, { "country", pres_country }, { "mandate_beginning", pres_mandate_beg }
                };

                auto president = TableInfo(false, 4, 0, pres_columns);

                CreateTable(fd, "president", president);

                auto page_id = ColumnInfo(DB_INT_SIZE, false);

                auto page_ns = ColumnInfo(DB_INT8_SIZE, false);

                auto page_title = ColumnInfo(DB_STRING_SIZE, false);

                auto page_revision_id = ColumnInfo(DB_INT_SIZE, false);

                auto page_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", page_id }, { "ns", page_ns }, { "title", page_title }, { "revision_id", page_revision_id }
                };

                auto pages = TableInfo(false, 4, 0, page_columns);

                CreateTable(fd, "pages", pages);

                auto revision_id = ColumnInfo(DB_INT_SIZE, false);

                auto revision_parent_id = ColumnInfo(DB_INT_SIZE, false);

                auto revision_timestamp = ColumnInfo(DB_INT64_SIZE, false);

                auto revision_contributor_id = ColumnInfo(DB_INT_SIZE, false);

                auto revision_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", revision_id },
                    { "parent_id", revision_parent_id },
                    { "timestamp", revision_timestamp },
                    { "contributor_id", revision_contributor_id },
                };

                auto revisions = TableInfo(false, 4, 0, revision_columns);

                CreateTable(fd, "revisions", revisions);

                auto contr_id = ColumnInfo(DB_INT_SIZE, false);

                auto contr_username = ColumnInfo(DB_STRING_SIZE, false);

                auto contr_columns = std::vector<std::pair<std::string, ColumnInfo>> {
                    { "id", contr_id },
                    { "username", contr_username },
                };

                auto contributors = TableInfo(false, 2, 0, contr_columns);

                CreateTable(fd, "contributors", contributors);

                auto ns_key = ColumnInfo(DB_INT_SIZE, false);

                auto ns_name = ColumnInfo(DB_STRING_SIZE, false);

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

#ifdef _GLIBCXX_DEBUG_ONLY
        PrintIndex(std::cout);
#endif
    }

    auto Run() -> void
    {
        std::string input;

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
                } else if (input == ".insert_data") {
                    std::cout << "Insertion des data\n";
                    import_all_csv();
                } else if (input == ".print_table_layout") {
                    PrintIndex(std::cout);
                } else {
                    Utils::Repl::Print(Eval(input));
                }

                // Add to history
                replxx_history_add(rx, input.c_str());
            }
        } else {

            if (Settings.m_Address == "") {
                throw Errors::Error(Errors::ErrorType::CLIArgument, "Use of --serve / -s requires an address", 0, 0, Errors::ERROR_UNGIVEN_ARGUMENT);
            }

            const std::string& delimiter = ":";

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
        }
    }

    ~DatabaseEngine()
    {
        Cleanup();
    }

    void import_all_csv();
    void process_csv_streaming(const std::string& path, const std::string& table, const std::vector<std::string>& columns);
};
}

#endif // !DATA
