#include "database.h"
#include "algebrizer/algebrizer.h"
#include "data_process_system/racine.h"
#include "storage/record.h"

#include <fstream>
#include <numeric>
#include <ostream>

namespace Database {

auto DatabaseEngine::ParseArguments(int argc, char** argv) -> DatabaseSetting*
{
    auto Settings = new DatabaseSetting();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--serve" || arg == "-s") {
            if (i + 1 < argc) {
                Settings->m_Repl = false;
                Settings->m_Address = argv[++i];
            } else {

                throw Errors::Error(Errors::ErrorType::CLIArgument, "Use of --serve / -s requires an address", 0, 0, Errors::ERROR_UNGIVEN_ARGUMENT);
            }
        } else if (arg == "--repl" || arg == "-r") {
            Settings->m_Repl = true;
        } else if (arg == "--file" || arg == "-f") {
            if (i + 1 < argc) {
                Settings->m_FileName = argv[++i];
            } else {

                throw Errors::Error(Errors::ErrorType::CLIArgument, "Use of --file / -f requires a file name", 0, 0, Errors::ERROR_UNGIVEN_ARGUMENT);
            }
        } else {
            throw Errors::Error(Errors::ErrorType::CLIArgument, "Given argument is not recognized", 0, 0, Errors::ERROR_UNKNOWN_ARGUMENT);
        }
    }

    return Settings;
}

auto DatabaseEngine::FindDBFile() -> const std::string
{
    std::string db_path;

    std::string ext = ".db";

    for (auto& p : std::filesystem::recursive_directory_iterator(std::filesystem::current_path())) {
        if (p.path().extension() == ext) {
            db_path = p.path();
        }
    }

    return db_path;
}

auto DatabaseEngine::InitializeSystemTables(int fd) -> void
{
    using namespace Storing;

    Cursor::SetOffset(SCHEMA_TABLE_OFFSET);

    // 1. Définition du type

    // System Table storing locations and info about DB's data
    // Stocke les données relatives aux colonnes.
    // d'un côté les tables et de l'autre les colonnes
    //
    // -- Schema Table --
    // Text name ->  name ('table')
    // Bool is_sys
    // Int16 current_element_nb
    // Int8 column_number
    // Int column_offset -> first column offset (assuming they are all aligned)

    uint32_t first_schema_table_offset = Cursor::MoveOffset(MAX_TABLE * DB_STRING_SIZE);

    ColumnInfo* t_name = new ColumnInfo(first_schema_table_offset, DB_STRING_SIZE, false);

    ColumnInfo* is_sys = new ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * DB_BOOL_SIZE), DB_BOOL_SIZE, false);

    ColumnInfo* current_element_nb = new ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * DB_INT16_SIZE), DB_INT16_SIZE, false);

    ColumnInfo* column_number = new ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * DB_INT8_SIZE), DB_INT8_SIZE, false);

    ColumnInfo* column_offset = new ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * DB_INT_SIZE), DB_INT_SIZE, false);

    auto table_args = std::vector<std::pair<std::string, ColumnInfo>> {
        { "name", *t_name },
        { "is_sys", *is_sys },
        { "current_max_record", *current_element_nb },
        { "column_number", *column_number },
        { "column_offset", *column_offset }
    };

    TableInfo* schema_table = new TableInfo(
        true, table_args.size(), first_schema_table_offset, table_args);

    // -- Schema Column --
    // Text name ->  name ('column')
    // Int offset -> col's beginning location in memory
    // Int8 element_size -> size of an element
    // Bool is_sortable
    // Bool is_sorted
    // Bool is_compressable
    // Bool is_compressed

    // Keeping the first offset to initialise the TableInfo class later
    uint32_t first_schema_column_offset = Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_STRING_SIZE);
    ColumnInfo* name = new ColumnInfo(first_schema_column_offset, DB_STRING_SIZE, false);

    ColumnInfo offset = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_INT_SIZE),
        DB_INT_SIZE, false);

    ColumnInfo element_size = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_INT8_SIZE),
        DB_INT8_SIZE, false);

    ColumnInfo is_sortable = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_BOOL_SIZE),
        DB_BOOL_SIZE, false);

    ColumnInfo is_sorted = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_BOOL_SIZE),
        DB_BOOL_SIZE, false);

    ColumnInfo is_compressable = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_BOOL_SIZE),
        DB_BOOL_SIZE, false);

    ColumnInfo is_compressed = ColumnInfo(
        Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_BOOL_SIZE),
        DB_BOOL_SIZE, false);

    auto column_args = std::vector<std::pair<std::string, ColumnInfo>> {
        { "name", *name },
        { "offset", offset },
        { "element_size", element_size },
        { "sortable", is_sortable },
        { "sorted", is_sorted },
        { "compressable", is_compressable },
        { "compressed", is_compressed }
    };

    TableInfo* schema_column = new TableInfo(
        true, column_args.size(), first_schema_column_offset, column_args);

    // 2. Ajout des tables à l'index

    Index->insert({ "schema_table", *schema_table });
    Index->insert({ "schema_column", *schema_column });

    // 3. Update du compte de table en mémoire

    File::IncrTableCount(fd);
    File::IncrTableCount(fd);

    // 4. Ecriture dans le header du fichier d'un prototype pointant vers la
    // table elle-même

    std::cout << "Writing Columns of Column Table...\n";

    for (auto e : schema_column->m_Columns) {

        Record::Write(fd, &Index->at("schema_column"), &e.second, e.first);
    }

    std::cout << "Writing Columns of Table Table...\n";

    for (auto e : schema_table->m_Columns) {

        Record::Write(fd, &Index->at("schema_column"), &e.second, e.first);
    }

    std::cout << "Writing Tables of Table Table and Column Table...\n";

    // Create a special Write overload to handle this op
    // Cause they have special parameters (especially column offsets I have to
    // get)

    Record::Write(fd, &Index->at("schema_table"), schema_column,
        "schema_column");

    Record::Write(fd, &Index->at("schema_table"), schema_table, "schema_table");

    // Add them to this global vector that will remember in which order they are stored in memory
    TableOrder.push_back("schema_column");

    TableOrder.push_back("schema_table");
}

// Crée en mémoire la représentation de la table pays
auto DatabaseEngine::CreateCountryTable(int fd) -> void
{
    using namespace Database::Storing;

    // -- Country Table --
    // Text name
    // Int pop

    DbInt first_offset = Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE);

    // Passer comme offset la première localisation encore disponible
    ColumnInfo* c_name = new ColumnInfo(first_offset, DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_name, "name");

    ColumnInfo* c_pop = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
        DB_INT_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_pop, "pop");

    auto c_columns = std::vector<std::pair<std::string, ColumnInfo>> {
        { "name", *c_name }, { "pop", *c_pop }
    };

    // 3. Write it on disk in the dedicated system table

    auto country = new TableInfo(false, 2, first_offset, c_columns);

    Index->insert({ "country", *country });

    Record::Write(fd, &Index->at("schema_table"), country, "country");

    TableOrder.push_back("country");

    File::IncrTableCount(fd);
}

// Crée en mémoire la représentation de la table pays
auto DatabaseEngine::CreateCityTable(int fd) -> void
{
    using namespace Database::Storing;

    // -- City Table --
    // Text name
    // Int pop
    // Text country

    DbInt first_offset = Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE);

    // Passer comme offset la première localisation encore disponible
    ColumnInfo* c_name = new ColumnInfo(first_offset, DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_name, "name");

    ColumnInfo* c_pop = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
        DB_INT_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_pop, "pop");

    ColumnInfo* c_country = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE),
        DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_country, "country");

    auto c_columns = std::vector<std::pair<std::string, ColumnInfo>> {
        { "name", *c_name }, { "pop", *c_pop }, { "country", *c_country }
    };

    // 3. Write it on disk in the dedicated system table

    auto city = new TableInfo(false, 3, first_offset, c_columns);

    Index->insert({ "city", *city });

    Record::Write(fd, &Index->at("schema_table"), city, "city");

    TableOrder.push_back("city");

    File::IncrTableCount(fd);
}

auto DatabaseEngine::CreatePresidentTable(int fd) -> void
{
    using namespace Database::Storing;

    // -- President Table --
    // Text first_name
    // Text last_name
    // Text country
    // Int mandate_beginning

    DbInt first_offset = Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE);

    // Passer comme offset la première localisation encore disponible
    ColumnInfo* c_first_name = new ColumnInfo(first_offset, DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_first_name, "first_name");

    ColumnInfo* c_last_name = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_last_name, "last_name");

    ColumnInfo* c_country = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_STRING_SIZE), DB_STRING_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_country, "country");

    ColumnInfo* c_mandate_beg = new ColumnInfo(Cursor::MoveOffset(MAX_ELEMENT_PER_COLUMN * DB_INT_SIZE),
        DB_INT_SIZE, false);

    Record::Write(fd, &Index->at("schema_column"), c_mandate_beg, "mandate_beginning");

    auto c_columns = std::vector<std::pair<std::string, ColumnInfo>> {
        { "first_name", *c_first_name }, { "last_name", *c_last_name }, { "country", *c_country }, { "mandate_beginning", *c_mandate_beg }
    };

    // 3. Write it on disk in the dedicated system table

    auto president = new TableInfo(false, 4, first_offset, c_columns);

    Index->insert({ "president", *president });

    Record::Write(fd, &Index->at("schema_table"), president, "president");

    TableOrder.push_back("president");

    File::IncrTableCount(fd);
}

auto DatabaseEngine::FillIndex() -> void
{
    using namespace Database::Storing;

    int fd = File->Fd();

    std::cout << "Creating index with file contained data...\n";
    uint8_t table_number = Storing::File::GetTableCount(fd);

    std::cout << "Found " << static_cast<int>(table_number) << " tables.\n";
    lseek(fd, SCHEMA_TABLE_OFFSET, SEEK_SET);

    std::vector<DbString> name = {};
    std::vector<DbBool> is_sys = {};
    std::vector<DbInt16> current_element_nb = {};
    std::vector<DbInt8> col_num = {};
    std::vector<DbInt> col_offsets = {};

    std::cout << "Allocating index memory...\n";

    DbInt64 offset = SCHEMA_TABLE_OFFSET;

    FileInterface::ReadVec(fd, name, &offset, DB_STRING_SIZE, table_number);

    offset += (MAX_TABLE - table_number) * DB_STRING_SIZE;

    FileInterface::ReadVec(fd, is_sys, &offset, DB_BOOL_SIZE, table_number);

    offset += (MAX_TABLE - table_number) * DB_BOOL_SIZE;

    FileInterface::ReadVec(fd, current_element_nb, &offset, DB_INT16_SIZE, table_number);

    offset += (MAX_TABLE - table_number) * DB_INT16_SIZE;

    FileInterface::ReadVec(fd, col_num, &offset, DB_INT8_SIZE, table_number);

    offset += (MAX_TABLE - table_number) * DB_INT8_SIZE;

    FileInterface::ReadVec(fd, col_offsets, &offset, DB_INT_SIZE, table_number);

    auto column_data = ColumnInfo::GetColumnsData(
        fd, std::accumulate(col_num.begin(), col_num.end(), 0));

    std::cout << "Filling Index...\n";
    for (int i = 0; i < table_number; i++) {

        auto info = new TableInfo(
            is_sys[i], current_element_nb[i], col_num[i], col_offsets[i],
            *TableInfo::GetTableColumns(
                fd, col_num[i],
                std::accumulate(col_num.begin(), col_num.begin() + i, 0),
                column_data));

        const std::string converted_name = Convert::DbStringToString(name[i]);

        TableOrder.push_back(converted_name);

        Index->insert({ converted_name,
            *info });
    }

    std::cout << "Done.\n";
}

auto DatabaseEngine::Eval(const std::string& input) -> const std::string
{

    std::string output = "";

    Parsing::Parser* parser = new Parsing::Parser(input);

    std::variant<Parsing::Statement, Errors::Error> n = parser->Parse();

    if (std::holds_alternative<Errors::Error>(n)) {
        Errors::Error e = std::get<Errors::Error>(n);

        throw e;
    }

    auto stmt = std::get<Parsing::Statement>(n);

    // Traiter chaque Statement
    if (std::holds_alternative<Parsing::SelectStmt*>(stmt)) {
        auto select = std::get<Parsing::SelectStmt*>(stmt);

        auto joins = select->getJoins();

        QueryPlanning::ConversionEnArbre_ET_excution(select, File, Index.get());

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

            auto err = Storing::Store::DB_SetData(File->Fd(), Index.get(), name, *data);

            delete data;

            if (err.has_value()) {
                throw err;
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
                std::span(col_data->begin() + col_number * i, col_data->begin() + (col_number * i + col_number)),
                col_order);

            auto err = Storing::Store::DB_SetData(File->Fd(), Index.get(), name, *data);

            if (err.has_value()) {
                throw err;
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

        throw Errors::Error(Errors::ErrorType::RuntimeError, "Wait this is illegal", 0, 0, Errors::ERROR_WRONG_MEMORY_ACCESS);
    }

    delete parser;

    return output;
}

void DatabaseEngine::process_csv_streaming(const std::string& path, const std::string& table, const std::vector<std::string>& columns)
{
    constexpr int MAX_ROWS_PER_TRANSACTION = 100;
    const size_t ncols = columns.size();
    int compteur = 0;

    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    std::string header;
    std::getline(in, header); // skip header

    std::vector<std::string> batch;
    batch.reserve(MAX_ROWS_PER_TRANSACTION);

    std::string line;
    while (std::getline(in, line)) {
        compteur++;
        batch.push_back(line.substr(0, line.size() - 1));

        if ((int)batch.size() >= MAX_ROWS_PER_TRANSACTION) {

            // std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // Process this batch
            std::ostringstream query;
            query << "TRANSACTION " << table << " (";
            for (size_t i = 0; i < ncols; ++i) {
                if (i)
                    query << ", ";
                query << columns[i];
            }
            query << ") VALUES ";
            bool peut_ajouter_virgule = false;
            for (size_t j = 0; j < batch.size(); j++) {

                std::stringstream ss(batch[j]);
                std::string cell;
                std::vector<std::string> vals;
                size_t pos = ss.str().find("\"");

                if (pos == std::string::npos) {
                    if (j != 0 && peut_ajouter_virgule)
                        query << ",";
                    peut_ajouter_virgule = true;
                    while (std::getline(ss, cell, ','))
                        vals.push_back(cell);

                    query << "(";
                    for (size_t k = 0; k < vals.size(); ++k) {
                        if (k)
                            query << ", ";
                        auto& v = vals[k];
                        if (v.empty() || v == "NULL")
                            query << "0";
                        else {
                            bool numeric = true;
                            for (char c : v)
                                if (!std::isdigit(c)) {
                                    numeric = false;
                                    break;
                                }
                            if (numeric)
                                query << v;
                            else {
                                query << "\"" << v << "\"";
                            }
                        }
                    }
                    query << ")";
                }
            }
            query << " END;";

            if (compteur > 2000000)
                break;

            DatabaseEngine::Eval(query.str());
            batch.clear();
        }
    }
    // Handle the final partial batch
    if (!batch.empty()) {
        // Process this batch
        std::ostringstream query;
        query << "TRANSACTION " << table << " (";
        for (size_t i = 0; i < ncols; ++i) {
            if (i)
                query << ", ";
            query << columns[i];
        }
        query << ") VALUES ";
        bool peut_ajouter_virgule = false;
        for (size_t j = 0; j < batch.size(); j++) {

            std::stringstream ss(batch[j]);
            std::string cell;
            std::vector<std::string> vals;
            size_t pos = ss.str().find("\"");

            if (pos == std::string::npos) {
                if (j != 0 && peut_ajouter_virgule)
                    query << ",";
                peut_ajouter_virgule = true;
                while (std::getline(ss, cell, ','))
                    vals.push_back(cell);

                query << "(";
                for (size_t k = 0; k < vals.size(); ++k) {
                    if (k)
                        query << ", ";
                    auto& v = vals[k];
                    if (v.empty() || v == "NULL")
                        query << "0";
                    else {
                        bool numeric = true;
                        for (char c : v)
                            if (!std::isdigit(c)) {
                                numeric = false;
                                break;
                            }
                        if (numeric)
                            query << v;
                        else {
                            query << "\"" << v << "\"";
                        }
                    }
                }
                query << ")";
            }
        }
        query << " END;";

        DatabaseEngine::Eval(query.str());
        batch.clear();
    }
}

void DatabaseEngine::import_all_csv()
{
    DatabaseEngine::process_csv_streaming("../script/table/contributor.csv", "contributors", { "id", "username" });
    DatabaseEngine::process_csv_streaming("../script/table/revision.csv", "revisions", { "id", "parent_id", "timestamp", "contributor_id" });
    DatabaseEngine::process_csv_streaming("../script/table/page.csv", "pages", { "id", "ns", "title", "revision_id" });

    // DatabaseEngine::process_csv_streaming("../script/table/namespaces.csv", "namespaces", { "key", "name" });
    //  DatabaseEngine::process_csv_streaming("../script/table/categories_pages.csv", "categories_pages", { "id_cat", "page_id" });
    //  DatabaseEngine::process_csv_streaming("../script/table/categories.csv", "categories", { "id", "name" });
}

auto DatabaseEngine::PrintIndex(std::ostream& out) -> void
{

    if (Index == nullptr) {
        out << "Index was not already allocated.\n";
    }
    int fd = File->Fd();

    out << "\n\n--- File's Tables ---\n";

    for (auto e : *Index) {
        out << "Reading info from table: " << e.first << "\n";
        out << "Is sys: " << (e.second.IsSys() == 0 ? "false" : "true") << "\n";
        out << "Element number: " << e.second.GetElementNumber() << "\n";
        out << "Column number: " << static_cast<int>(e.second.GetColumnNumber())
            << "\n";

        for (auto c : e.second.m_Columns) {
            out << "Reading info from column: " << c.first << "\n";
            out << "Found at offset: 0x" << c.second.GetOffset()
                << "\n";
            out << "Element size: " << static_cast<int>(c.second.GetElementSize())
                << "\n";
        }

        out << "\n";
    }
}
}
