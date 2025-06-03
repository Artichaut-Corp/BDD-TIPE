#include <cstdint>
#include <string>
#include <sys/types.h>

#include "types.h"

#ifndef CURSOR_H

#define CURSOR_H

namespace Database::Storing {

// Pourrait contenir des méthodes permettant de se déplacer aux
// différents offsets du fichier
class Cursor {

public:
    static void GotoBeginning(int fd);

    static void GotoColumn(int fd, std::string column_key);

    static void SetOffset(uint64_t size);

    static uint64_t CurrentOffset();

    static uint64_t MoveOffset(uint64_t size);
};

}

#endif // !CURSOR_H
