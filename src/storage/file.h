#include <array>
#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <unistd.h>
#include <variant>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../errors.h"

#include "column.h"
#include "cursor.h"
#include "record.h"
#include "table.h"
#include "types.h"

#ifndef FILE_H

#define FILE_H

#define MAX_PAGES 4096
#define DEFAULT_PAGE_SIZE 4096

namespace Database::Storing {

constexpr uint8_t signature[52] = {
    0x64, 0x65, 0x20, 0x63, 0x68, 0x61, 0x63, 0x75, 0x6E, 0x20, 0x73,
    0x65, 0x6C, 0x6F, 0x6E, 0x20, 0x73, 0x65, 0x73, 0x20, 0x6D, 0x6F,
    0x79, 0x65, 0x6E, 0x73, 0x20, 0x61, 0x20, 0x63, 0x68, 0x61, 0x63,
    0x75, 0x6E, 0x20, 0x73, 0x65, 0x6C, 0x6F, 0x6E, 0x20, 0x73, 0x65,
    0x73, 0x20, 0x62, 0x65, 0x73, 0x6F, 0x69, 0x6E
};

class Pager {
private:
    int m_FileDescriptor;
    uint32_t m_FileLength;

    std::array<void*, MAX_PAGES> m_Pages;

public:
    Pager(int fd, uint32_t lenght)
        : m_FileDescriptor(fd)
        , m_FileLength(lenght)
    {
        for (uint32_t i = 0; i < MAX_PAGES; i++) {
            this->m_Pages[i] = nullptr;
        }
    }

    ~Pager()
    {
        close(m_FileDescriptor);
    }

    static std::variant<Pager*, Errors::Error> OpenPager(const std::string& filepath);

    std::variant<void*, Errors::Error> GetPage(uint32_t pageNumber);
};

class File {

private:
    int m_Fd;

    int m_Size;

    std::unique_ptr<Cursor> m_Cursor;

public:
    File(std::string filepath)
    {
        int fd = open(filepath.c_str(), O_RDWR, S_IWUSR | S_IRUSR);

        std::cout << "Opening " << filepath << " ...\n";

        if (!RecognizedDatabaseSignature(fd)) {
            AddDatabaseSignature(fd);
        }

        struct stat s;

        if (fstat(fd, &s) == -1) {
            throw std::runtime_error("failed fstat");
        }

        m_Size = s.st_size;

        m_Fd = fd;
    }

    static int GetTableCount(int fd)
    {
        lseek(fd, TABLE_NUMBER_OFFSET, SEEK_SET);

        uint8_t buffer[1];

        read(fd, &buffer, 1);

        return buffer[0];
    }

    // Prépare et charge l'index des tables déjà présentes en mémoire
    static void FillIndex(int fd)
    {
        uint8_t table_number = GetTableCount(fd);

        lseek(fd, SCHEMA_TABLE_OFFSET, SEEK_SET);

        std::vector<std::array<uint8_t, 255>> name = {};
        std::vector<bool> is_sys = {};
        std::vector<uint32_t> curr_max_record = {};
        std::vector<uint32_t> col_num = {};
        std::vector<std::vector<uint32_t>> col_offsets = {};

        name.reserve(table_number);
        is_sys.reserve(table_number);
        curr_max_record.reserve(table_number);
        col_num.reserve(table_number);

        int offset = SCHEMA_TABLE_OFFSET;

        lseek(fd, offset, SEEK_SET);

        read(fd, &is_sys, DB_STRING_SIZE * table_number);

        offset += 64 * DB_STRING_SIZE;

        lseek(fd, offset, SEEK_SET);

        read(fd, &is_sys, DB_BOOL_SIZE * table_number);

        offset += 64 * DB_BOOL_SIZE;

        lseek(fd, offset, SEEK_SET);

        read(fd, &curr_max_record, DB_INT_SIZE * table_number);

        offset += 64 * DB_INT_SIZE;

        lseek(fd, offset, SEEK_SET);

        read(fd, &col_num, DB_INT_SIZE * table_number);

        lseek(fd, SCHEMA_COLUMN_OFFSET, SEEK_SET);
    }

    static bool RecognizedDatabaseSignature(int fd)
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

    static void AddDatabaseSignature(int fd)
    {
        int e = write(fd, signature, 52 * DB_CHAR_SIZE);
    }

    static std::string CreateFile()
    {

        std::string path = std::format(
            "{}/{}", std::filesystem::current_path().string(), "main.db");

        int fd = open(path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

        std::cout << "Created " << path << "\n";

        AddDatabaseSignature(fd);

        // Write System Tables
        InitializeSystemTables(fd, Index);

        close(fd);

        return path;
    }

    // A modifier pour assurer la validité de la variable
    int Fd() const { return m_Fd; }

    // Cette méthode n'est normalement appelée qu'une fois lors de la création du
    // fichier .db. Elle initialise les tables qui contiendront les méta-données
    // sur les futures tables
    static void InitializeSystemTables(int fd, DBTableIndex& index)
    {

        std::cout << "Starting to initialize system tables...\n";

        Cursor::SetOffset(SCHEMA_TABLE_OFFSET);

        // 1. Définition du type

        // System Table storing locations and info about DB's data
        // Stocke les données relatives aux colonnes.
        // d'un côté les tables et de l'autre les colonnes
        //
        // -- Schema Table --
        // Text name ->  name ('table')
        // Int is_sys
        // Int current_element_nb
        // Int column_number
        // Int[] offsets -> array of Schema Column positions in header (first
        // element offset)

        ColumnInfo* t_name = new ColumnInfo(

            Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_STRING_SIZE),
            DB_STRING_SIZE, 0, false);

        ColumnInfo* is_sys = new ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_BOOL_SIZE),
            DB_BOOL_SIZE, 0, false);

        ColumnInfo* current_element_nb = new ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_INT_SIZE),
            DB_INT_SIZE, 0, false);

        ColumnInfo* column_number = new ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * MAX_COLUMN_PER_TABLE * DB_INT_SIZE),
            DB_INT_SIZE, 0, false);

        ColumnInfo* t_offsets = new ColumnInfo(Cursor::MoveOffset(MAX_TABLE * DB_INT_ARRAY_SIZE * MAX_COLUMN_PER_TABLE),
            DB_INT_ARRAY_SIZE, 0, false);

        auto table_args = std::vector<std::pair<std::string, ColumnInfo>> {
            { "name", *t_name },
            { "is_sys", *is_sys },
            { "current_max_record", *current_element_nb },
            { "column_number", *column_number },
            { "offsets", *t_offsets }
        };

        TableInfo* schema_table = new TableInfo(true, 0, table_args.size(), table_args);

        // -- Schema Column --
        // Text name ->  name ('column')
        // Int offset -> col's beginning location in memory
        // Int element_size -> size of an element
        // Int is_sortable
        // Int is_sorted
        // Int is_compressable
        // Int is_compressed

        ColumnInfo* name = new ColumnInfo(Cursor::MoveOffset(MAX_TABLE * DB_STRING_SIZE),
            DB_STRING_SIZE, 0, false);

        ColumnInfo offset = ColumnInfo(Cursor::MoveOffset(MAX_TABLE * DB_INT_SIZE),
            DB_INT_SIZE, 0, false);

        ColumnInfo element_size = ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_INT_SIZE), DB_INT_SIZE, 0, false);

        ColumnInfo* current_max_record = new ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_INT_SIZE), DB_INT_SIZE, 0, false);

        ColumnInfo is_sortable = ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_BOOL_SIZE), DB_BOOL_SIZE, 0, false);

        ColumnInfo is_sorted = ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_BOOL_SIZE), DB_BOOL_SIZE, 0, false);

        ColumnInfo is_compressable = ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_BOOL_SIZE), DB_BOOL_SIZE, 0, false);

        ColumnInfo is_compressed = ColumnInfo(
            Cursor::MoveOffset(MAX_TABLE * DB_BOOL_SIZE), DB_BOOL_SIZE, 0, false);

        auto column_args = std::vector<std::pair<std::string, ColumnInfo>> {
            { "name", *name },
            { "offset", offset },
            { "element_size", element_size },
            { "current_max_record", *current_max_record },
            { "sortable", is_sortable },
            { "sorted", is_sorted },
            { "compressable", is_compressable },
            { "compressed", is_compressed }
        };

        TableInfo* schema_column = new TableInfo(true, 0, column_args.size(), column_args);

        // 2. Ajout des tables à l'index

        index.insert({ "schema_table", *schema_table });
        index.insert({ "schema_column", *schema_column });

        // 3. Ecriture dans le header du fichier d'un prototype pointant vers la
        // table elle-même

        std::cout << "Writing columns of column table...\n";

        std::vector<DbInt>* column_table_offsets = new std::vector<DbInt>();

        for (auto e : schema_column->m_Columns) {
            column_table_offsets->push_back(

                Record::Write(fd, index.at("schema_column"), &e.second, e.first));
        }

        std::cout << "Writing columns of table table...\n";

        std::vector<DbInt>* table_table_offsets = new std::vector<DbInt>();

        for (auto e : schema_table->m_Columns) {
            table_table_offsets->push_back(
                Record::Write(fd, index.at("schema_column"), &e.second, e.first));
        }

        std::cout << "Writing tables of table table and column table...\n";

        // Create a special Write overload to handle this op
        // Cause they have special parameters (especially column offsets I have to
        // get)
        Record::Write(fd, index.at("schema_table"), schema_table, "schema_table",
            *table_table_offsets);

        Record::Write(fd, index.at("schema_table"), schema_column, "schema_column",
            *column_table_offsets);

        std::cout << "Finished to initialize system tables.\n\n";
    }
};

}

#endif // !FILE_H
