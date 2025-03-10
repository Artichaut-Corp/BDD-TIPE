#include <string>
#include <vector>

#ifndef DDL_H

#define DDL_H

namespace Database::Parsing {

// Type générique, j'imagine qu'on aura un dictionnaire regroupant toutes les
// options
class DatabaseOption {
    bool m_Activated;

    // probablement créer une enum contenant ce qu'on autorisera
    int m_Code;
};

enum class ConstraintType { UNIQUE,
    NOT_NULL,
    FOREIGN_KEY };

class AlterTableSpec { };
class ColumnDef { };

// Statements permettant de définir la classure de la DB (DDL)
class AlterTableStmt {
    std::string m_Name;

    // Liste des options activées
    std::vector<DatabaseOption*> m_Options;
};

class CreateDatabaseStmt {
    std::string m_Name;

    // Liste des options activées
    std::vector<DatabaseOption*> m_Options;
};

class DropDatabaseStmt {
    std::string m_Name;
};

// TODO: Implémenter ces 4 stmt et tous leurs éventuels sous-champ
class CreateTableStmt {
    // Nom de la table
    // Quelle DB
    // Noms de chaque colonne dans la table
    // Le type de chaque colonne
    // Une valeur par défaut
    // Optionnel: Clé primaire
    // Un set de contraintes
};

class DropTableStmt {
    std::string m_TableName;
};

class RenameTableStmt {
    std::string m_TableName;
};

} // namespace parsing

#endif
