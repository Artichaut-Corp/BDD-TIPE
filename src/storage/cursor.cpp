#include "cursor.h"

namespace Database::Storing {

void Cursor::GotoBeginning(int fd) { m_CurrOffset = HEADER_OFFSET; }

void Cursor::GotoColumn(int fd, std::string column_key) { }

void Cursor::SetOffset(DbUInt64 size) { m_CurrOffset = size; }

DbUInt64 Cursor::CurrentOffset() { return m_CurrOffset; }

DbUInt64 Cursor::MoveOffset(DbUInt64 size)
{
    DbUInt64 current = m_CurrOffset;

    m_CurrOffset += size;

    return current;
}

} // namespace Database::Storing
