#ifndef DATABASE_H

#define DATABASE_H

#include "errors.h"
#include "repl.h"
#include "storage/pager.h"

#include <filesystem>
#include <memory>
#include <variant>

namespace Database {

enum class DatabaseOptions {

};

class DatabaseI {

    std::unique_ptr<Storing::Pager> m_Pager;

public:
    DatabaseI(Storing::Pager* p)
        : m_Pager(std::unique_ptr<Storing::Pager>(p))
    {
    }

    static void Init(int argc, char* argv[])
    {

        std::string db_path;

        // If there is no arguments, then it's a line by line interactive interpreter
        // Running default *.db database
        if (argc < 2) {
            std::string ext = ".db";

            for (auto& p : std::filesystem::recursive_directory_iterator(std::filesystem::current_path())) {
                if (p.path().extension() == ext) {
                    db_path = p.path();
                }
            }

        } else {
            db_path = argv[1];
        }

        auto p = Storing::Pager::OpenPager(db_path);

        if (std::holds_alternative<Errors::Error>(p)) {
            std::get<Errors::Error>(p).printAllInfo();

            return;
        }

        Storing::Pager* pager = std::get<Storing::Pager*>(p);

        Utils::Repl::Run();
    }
};

}

#endif // !DATABASE_H
