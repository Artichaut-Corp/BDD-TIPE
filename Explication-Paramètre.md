SelectionDescent -> si ce paramètre vaut 1, les selections sont descendu le plus possible dans le plan

PronfMode -> si il vaut 0 le join éxécuté dans tout les cas est le produit cartésien, si il vaut 2 c'est un pré-tri des colonne , si il vaut 3, il utilise une map

InsertProj -> si il vaut 1 : Insère des projection avant chaque selection et jointure, pour minimiser le plus possible les élément manipuler à chaque étape de l'éxécution

OptimizeBinaryExpression -> si il vaut 1 -> ordonne les comparaisons dans les séléctions en fonction de la séléctivité de chacune des comparaison, utilisant le côté parresseux d'une condition
