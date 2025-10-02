#include "errors.h"
#include "parser.h"
#include "repl.h"
#include "server.h"
#include "storage.h"

#include <filesystem>
#include <format>
#include <memory>
#include <ostream>
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

    // Crée en mémoire la représentation de la table pays
    auto CreateCountryTable(int fd) -> void;

    auto CreateCityTable(int fd) -> void;

    auto CreatePresidentTable(int fd) -> void;

    // Prépare et charge l'index des tables déjà présentes en mémoire
    auto FillIndex() -> void;

    auto Eval(const std::string& input) -> const std::string;

    auto PrintIndex(std::ostream& out) -> void;

    auto Cleanup() -> void
    {
        int fd = File->Fd();

        int table_count = Storing::File::GetTableCount(fd);

        auto tables_element_count = std::vector<DbInt16>();

        tables_element_count.reserve(table_count);

        // Get all element_count from Index
        for (const std::string& table : TableOrder) {
            uint16_t element_count = Index->at(table).GetElementNumber();

            tables_element_count.emplace_back(element_count);
        }

        // Overwrite those values at the right place

        uint32_t offset = SCHEMA_TABLE_OFFSET + MAX_TABLE * (DB_STRING_SIZE + DB_BOOL_SIZE);

        lseek(fd, offset, SEEK_SET);

        DbInt16 buffer;

        for (int i = 0; i < table_count; i++) {

            buffer = tables_element_count[i];

            int bytes_written = write(fd, &buffer, DB_INT16_SIZE);
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

                // Create new table and insert some data
                CreateCountryTable(fd);

                CreateCityTable(fd);

                CreatePresidentTable(fd);

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

            for (;;) {
                std::cout << "SQL>>" << std::endl;

                input = Utils::Repl::Read();

                if (input == "\0") {
                    std::cout << "Exiting...\n";
                    m_Quit = true;

                    break;
                }

                Utils::Repl::Print(Eval(input));
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
};

}

#endif // !DATA
