#include <cstdio>
#include <string>

#include "storage/types.h"

namespace Database::Querying {

std::string getColumnTypeName(const ColumnData& col);

void afficherColumnData(const ColumnData& col);

bool operator==(const ColumnData& larg, const ColumnData& rarg);

bool operator!=(const ColumnData& larg, const ColumnData& rarg);

bool operator<(const ColumnData& larg, const ColumnData& rarg);

bool operator<=(const ColumnData& larg, const ColumnData& rarg);

bool operator>=(const ColumnData& larg, const ColumnData& rarg);

bool operator>(const ColumnData& larg, const ColumnData& rarg);

};
