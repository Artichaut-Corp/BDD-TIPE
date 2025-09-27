#include <map>
#include <optional>
#include <variant>
#include <vector>
#include "../parser.h"
#include "../algebrizer_types.h"

#ifndef BINARYTREE_H

#define BINARYTREE_H

namespace Database::QueryPlanning {

class Var {
private:
    std::variant<ColumnData,std::string> RightValue ; //ce serait bien que le nom des colonnes soit de la forme Table.Nom
    std::variant<ColumnData,std::string> LeftValue ;
    Database::Parsing::LogicalOperator Operation;
public:
    std::vector<std::string> GetColumnUsed (); //permet d'avoir le nom des colonne utilisé
    bool evalue ();
};

class BinaryTree {
private:
    std::string Operator ; //est un "AND" ou un "OR"
    std::optional<std::variant<Var*,BinaryTree*>> LeftChild;
    std::optional<std::variant<Var*,BinaryTree*>> RightChild; //si ni fils droit ni fils gauche, c'est un arbre toujours Vrai, ce cas de base peut être utile lors des extraction
    std::vector<std::string> ColonneUsed;//les noms de toute les colonne utilisé pour ce type binaire
public:
    std::vector<std::string> CreateTableUsedVector (); //faire un parcours postfix pour récuperer les colonnes des enfants, puis faire l'union des deux, l'enregistrer pour ce type puis la renvoyer pour l'appel récursif
    
    BinaryTree* ExtraireTable(std::vector<std::string> ColonneAExtraire); 
    //on séparer l'expression binaire en deux, celle qui se sert des tables passé en paramètre (c'est celle qu'on revoie) et celle qui n'ont pas besoin de ces tables pour s'évaluer (l'abre qui est là)

    bool Eval(std::map<std::string,ColumnData> CombinaisonATester);

    
};
};
#endif // ! BINARYTREE_H  BINARYTREE_H
