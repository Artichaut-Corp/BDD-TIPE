#include "repl.h"

using namespace Compiler;

int main(int argc, char* argv[])
{

    // If there is no arguments, then it's a line by line interactive interpreter
    if (argc < 2) {
        Utils::Repl::Run();
    }

    // else this is a whole file that will be interpreted

    return 0;
}
