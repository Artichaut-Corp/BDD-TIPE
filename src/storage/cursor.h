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

    static void SetOffset(DbUInt64 size);

    static DbUInt64 CurrentOffset();

    static DbUInt64 MoveOffset(DbUInt64 size);
};

}

#endif // !CURSOR_H
