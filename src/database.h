#include "errors.h"
#include "repl.h"
#include "storage.h"

#include <filesystem>
#include <memory>
#include <variant>

#ifndef DATABASE_H

#define DATABASE_H

namespace Database {

enum class DatabaseOptions {

};

class DatabaseI {

public:
    DatabaseI(Storing::File* p)
    {
    }

    static void Init(int argc, char* argv[])
    {

        std::string db_path;

        bool created = false;

        // If there is no arguments, then it's a line by line interactive interpreter
        // Running default *.db database
        if (argc < 2) {
            std::string ext = ".db";

            for (auto& p : std::filesystem::recursive_directory_iterator(std::filesystem::current_path())) {
                if (p.path().extension() == ext) {
                    db_path = p.path();
                }
            }

            if (db_path == "") {
                db_path = Storing::File::CreateFile();

                created = !created;
            }

        } else {
            db_path = argv[1];
        }

        Storing::File f = Storing::File(db_path, created);

        // Après avoir trouvé et chargé le fichier
        // On charge la table système qui repertorie toutes les colonnes dans
        // une HashMap / Array sous forme d'objets. En utilisant les infos
        // là-bas + les méthodes des Objets représentant les tuples on peut
        // ajouter des tuples ou aller chercher une colonne par ex

        /* std::unique_ptr<std::vector<ColumnData>> res =
            Record::GetColumn(f.Fd(), f.Index.at("country").GetColumnInfo("name")); */

        /*
         - Aller chercher une colonne

          1. Prendre la table:
          Table t = Index.at(table_name);

          std::vector<int> col_Data = t.GetColumn(column_name)
          // Qui appelle à un moment:
              Column payspop = db.at("pays__pop");

          - Ecrire un enregistrement dans une table

          Table t = Index.at(table_name);
          // Gérer les erreurs dans le cas où la table n'existe pas

          t.WriteRecord(table_name, )
      WriteRecord(
            "schema_table",
            t.MakeRecord(c1, c2, ...) );

          */

        Utils::Repl::Run(f);
    }
};

}

#endif // !DATABASE_H
