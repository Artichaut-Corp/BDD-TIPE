#include "cursor.h"

namespace Database::Storing {

void Cursor::GotoBeginning(int fd) { m_CurrOffset = HEADER_OFFSET; }

void Cursor::GotoColumn(int fd, std::string column_key) { }

void Cursor::SetOffset(uint64_t size) { m_CurrOffset = size; }

uint64_t Cursor::CurrentOffset() { return m_CurrOffset; }

uint64_t Cursor::MoveOffset(uint64_t size)
{
    uint64_t current = m_CurrOffset;

    m_CurrOffset += size;

    return current;
}

} // namespace Database::Storing
