#include "database.h"

using namespace Database;

int main(int argc, char* argv[])
{
    DatabaseEngine database;

    auto s = DatabaseEngine::ParseArguments(argc, argv);

    try {

        database.Run(s);

    } catch (const Errors::Error& e) {

        if (s->m_Repl)
            e.printAllInfo(std::cerr);
        else
            // Should send back to the server
            e.printAllInfo(std::cerr);
        return -1;
    }

    return 0;
}
