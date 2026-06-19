SelectionDescent -> si ce paramètre vaut 1, les sélections sont descendues le plus possible dans le plan

PronfMode -> si il vaut 0 le join exécuté dans tous les cas est le produit cartésien, s'il vaut 2, c'est un pré-tri des colonne, s'il vaut 3, il utilise une map

InsertProj -> si il vaut 1 : Insère des projections avant chaque sélection et jointure, pour minimiser le plus possible les éléments manipulés à chaque étape de l'exécution

OptimizeBinaryExpression -> s'il vaut 1 : ordonne les comparaisons dans les sélections en fonction de la sélectivité de chacune des comparaisons, utilisant le côté paresseux d'une condition

QueryJoinOrdering -> S'il vaut 1 on estime le rapport de cardinalité (RC : le nombre de lignes issu de la jointure divisé par le produit du nombre de lignes des deux tables) puis on effectue les jointures de la plus petite RC à la plus grande