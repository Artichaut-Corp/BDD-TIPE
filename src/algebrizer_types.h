#include <cstdio>
#include <string>

#include "storage/types.h"


#ifndef AlGEBRIZER_TYPE_H

#define AlGEBRIZER_TYPE_H

namespace Database::QueryPlanning {

std::string getColumnTypeName(const ColumnData& col);

void afficherColumnData(const ColumnData& col);

bool operator==(const ColumnData& larg, const ColumnData& rarg);

bool operator!=(const ColumnData& larg, const ColumnData& rarg);

bool operator<(const ColumnData& larg, const ColumnData& rarg);

bool operator<=(const ColumnData& larg, const ColumnData& rarg);

bool operator>=(const ColumnData& larg, const ColumnData& rarg);

bool operator>(const ColumnData& larg, const ColumnData& rarg);

};

#endif // ALGEBRIZER_TYPES_H