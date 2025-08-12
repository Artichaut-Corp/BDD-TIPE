#include "file.h"
#include "../storage.h"
#include "../utils.h"
#include "common.h"

#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <numeric>
#include <unistd.h>
#include <variant>
#include <vector>

namespace Database::Storing {

std::variant<Pager*, Errors::Error> Pager::OpenPager(const std::string& filepath)
{
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if (!File::RecognizedDatabaseSignature(fd)) {
        File::AddDatabaseSignature(fd);
    }

    if (fd == -1) {
        return Errors::Error(Errors::ErrorType::RuntimeError, std::format("File was not found with name {}", filepath), 0, 0, Errors::ERROR_FILE_NOT_FOUND);
    }

    off_t filelength = lseek(fd, 0, SEEK_END);

    return new Pager(fd, filelength);
}

std::variant<void*, Errors::Error> Pager::GetPage(uint32_t pageNumber)
{

    if (pageNumber > MAX_PAGES) {

        return Errors::Error(Errors::ErrorType::RuntimeError, "Tried to access unowned memory", 0, 0, Errors::ERROR_WRONG_MEMORY_ACCESS);
    }

    if (m_Pages[pageNumber] == nullptr) {
        void* page = malloc(DEFAULT_PAGE_SIZE);

        m_Pages[pageNumber] = page;
    }

    return m_Pages[pageNumber];
}

int File::GetTableCount(int fd)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    read(fd, &buffer, DB_INT8_SIZE);

    return buffer;
}

void File::SetTableCount(int fd, int value)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    buffer = value;

    write(fd, &buffer, DB_INT8_SIZE);
}

void File::IncrTableCount(int fd)
{
    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    uint8_t buffer;

    read(fd, &buffer, DB_INT8_SIZE);

    lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

    buffer++;

    write(fd, &buffer, DB_INT8_SIZE);
}

void File::PrintIndex(int fd, std::ostream& out)
{
    out << "\n\n--- File's Tables ---\n";

    for (auto e : Index) {
        out << "Reading info from table: " << e.first << "\n";
        out << "Is sys: " << e.second.IsSys() << "\n";
        out << "Element number: " << e.second.GetElementNumber() << "\n";
        out << "Column number: " << static_cast<int>(e.second.GetColumnNumber())
            << "\n";
        out << "Column table first offset: 0x" << e.second.GetColumnsFirstOffset()
            << "\n";

        for (auto c : e.second.m_Columns) {
            out << "Reading info from column: " << c.first << "\n";
        }

        out << "\n";
    }
}

void File::FillIndex(int fd)
{
    std::cout << "Creating index with file contained data...\n";
    uint8_t table_number = GetTableCount(fd);

    std::cout << "Found " << static_cast<int>(table_number) << " tables.\n";
    lseek(fd, SCHEMA_TABLE_OFFSET, SEEK_SET);

    std::vector<DbString> name = {};
    std::vector<DbBool> is_sys = {};
    std::vector<DbInt16> current_element_nb = {};
    std::vector<DbInt8> col_num = {};
    std::vector<DbInt> col_offsets = {};

    std::cout << "Allocating index memory...\n";

    name.reserve(table_number);
    is_sys.reserve(table_number);
    current_element_nb.reserve(table_number);
    col_num.reserve(table_number);
    col_offsets.reserve(table_number);

    int offset = SCHEMA_TABLE_OFFSET;

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

        Index.insert({ converted_name,
            *info });
    }

    std::cout << "Done.\n";
}

bool File::RecognizedDatabaseSignature(int fd)
{
    char buf[52];

    int e = read(fd, buf, 52 * DB_CHAR_SIZE);

    for (uint8_t i = 0; i < 52; i++) {

        if (buf[i] != signature[i]) {
            return false;
        }
    }
    return true;
}

void File::AddDatabaseSignature(int fd)
{
    int e = write(fd, signature, 52 * DB_CHAR_SIZE);
}

std::string File::CreateFile()
{

    std::string path = std::format(
        "{}/{}", std::filesystem::current_path().string(), "main.db");

    int fd = open(path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    AddDatabaseSignature(fd);

    SetTableCount(fd, 0);

    // Write System Tables
    InitializeSystemTables(fd);

    // Create new table and insert some data
    CountryRecord::CreateCountryTable(fd);

    // Clean up and close
    close(fd);

    return path;
}

void File::InitializeSystemTables(int fd)
{

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

    Index.insert({ "schema_table", *schema_table });
    Index.insert({ "schema_column", *schema_column });

    // 3. Update du compte de table en mémoire

    IncrTableCount(fd);
    IncrTableCount(fd);

    // 4. Ecriture dans le header du fichier d'un prototype pointant vers la
    // table elle-même

    std::cout << "Writing Columns of Column Table...\n";

    for (auto e : schema_column->m_Columns) {

        Record::Write(fd, &Index.at("schema_column"), &e.second, e.first);
    }

    std::cout << "Writing Columns of Table Table...\n";

    for (auto e : schema_table->m_Columns) {

        Record::Write(fd, &Index.at("schema_column"), &e.second, e.first);
    }

    std::cout << "Writing Tables of Table Table and Column Table...\n";

    // Create a special Write overload to handle this op
    // Cause they have special parameters (especially column offsets I have to
    // get)

    Record::Write(fd, &Index.at("schema_table"), schema_column,
        "schema_column");

    Record::Write(fd, &Index.at("schema_table"), schema_table, "schema_table");

    // Add them to this global vector that will remember in which order they are stored in memory
    TableOrder.push_back("schema_column");

    TableOrder.push_back("schema_table");
}

void File::Cleanup(int fd)
{

    int table_count = GetTableCount(fd);

    auto tables_element_count = std::vector<DbInt16>();

    tables_element_count.reserve(table_count);

    // Get all element_count from Index
    for (const std::string& table : TableOrder) {
        uint16_t element_count = Index.at(table).GetElementNumber();

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
}

} // namespace Database::Storing
