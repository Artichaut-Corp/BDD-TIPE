#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <variant>
#include <string>
namespace Database::Querying {

using DbNull = std::nullopt_t;
using DbInt = uint32_t;
using DbString = std::array<uint8_t, 255>;

using ColumnData = std::variant<DbNull, DbInt, DbString>;

std::string getColumnTypeName(const ColumnData& col);
void afficherColumnData(const ColumnData& col);
bool operator==(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
bool operator!=(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
bool operator<(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
bool operator<=(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
bool operator>=(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
bool operator>(const Database::Querying::ColumnData& larg, const Database::Querying::ColumnData& rarg);
};
