#include "b+tree.h"
#include "types.h"

#include <unistd.h>

namespace Database {

DbInt LeafSize(DbUInt8 e_size) { return DB_BOOL_SIZE + 2 * DB_UINT64_SIZE + (TREE_ORDER - 1) * e_size; }
DbInt InnerSize(DbUInt8 e_size) { return DB_BOOL_SIZE + 2 * DB_UINT8_SIZE + TREE_ORDER * DB_UINT64_SIZE + (TREE_ORDER - 1) * e_size; }

DbInt MaxNode(DbUInt8 e_size) { return MAX_ELEMENT_PER_COLUMN * std::max(LeafSize(e_size), InnerSize(e_size)); }

DbInt IndexReprSize(DbUInt8 e_size) { return INDEX_HEADER_SIZE + MaxNode(e_size); }

}
