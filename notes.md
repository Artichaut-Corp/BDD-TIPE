# Notes 'The design and implementation of modern column based databases'

Première idée 1970, utilisation commerciale à partir de 2000

## Comparaison en performance avec les classiques row bases databases
- NSM = N-ary Storage Model (Globalement la norme, tuple stockés les uns à la suite des autres)
- DSM = Decomposition Storage Model (une des premières implémentations de CBD, tous les 1ers éléments, puis tous les seconds, etc) 

DSM rend la lecture d'une seule colonne rapide, mais plusieurs colonnes ou une table entière, c'est bien + lent que NSM

## Architecture

3 architectures principalement

### [C-store](https://web.archive.org/web/20120305151916/http://db.lcs.mit.edu/projects/cstore/#papers)

- Données représentées par un ensemble de fichiers contenant les colonnes sont stockées compressées et ordonnées
- Système à 2 buffers, 1 optimisé pour la lecture (ROS), un pour l'écriture (WOS).  
  Régulièrement, on bouge les données du ROS vers le WOS.  
  C'est à ce moment qu'on compresse, ordonnes et écrit sous forme de colonne.
- Les colonnes peuvent apparaître rangées plusieurs fois dans plusieurs ordres
  On crée des groupes (projections) de colonnes rangées par attribut pour répondre à n'importe quelle requête 
- Les méthodes de compression peuvent être différentes pour chaques colonnes.
- architecture "no overwrite" = pas de 'UPDATE' mais 'DELETE' puis 'INSERT'
- Une requête regroupe donc l'accès à ROS et WOS
- Un tas de méthode d'optimisation: Jointures, [Late Materialization](https://ceur-ws.org/Vol-3130/paper3.pdf), 'batch processing'



### [MonetDB](https://www.monetdb.org/documentation-Aug2024/dev-guide/monetdb-internals/design-overview/)

- Données stockées colonne par colonne
- Pas de buffer 
- Requêtes exécutées sous forme d'opérations simples, les unes à la suite des autres, colonne par colonne.  
  **Préfère exploiter le CPU en compressant/décompressant à tour de bras plutôt que de faire beaucoup d'opérations I/O**  
  En effet, si les processeur ont gagné beaucoup de performances ces dernières années, la vitesse de lecture des disques  
  est restée plutôt similaire. On préfère donc passer le travail au CPU plutôt que de réaliser beaucoup de lectures/écriture
- [Algèbre BAT](https://www.researchgate.net/figure/MonetDB-a-BAT-Algebra-Machine_fig2_220538804) utilisée pour gérer les étapes de la requête
- Techniques d'optimisation 'Bulk Processing', [Late Materialization](https://ceur-ws.org/Vol-3130/paper3.pdf)


### Vector-Wise
- MonetDB est désavantagée par le fait que chaque opération charge une colonne entière en mémoire pour chaque opération.  
  Ce qui peut être compliqué, voire dangereux lorsque la taille de la colonne commence à atteindre celle de la RAM
  Pour régler ce problème, il faut gérer les colonnes par vecteurs de la taille du cache du CPU pour maximiser  
  l'optimisation.

## Techniques d'optimisation


- Pour information, un papier décrivant la méthode classique [Tuple-at-a-time](https://dl.acm.org/doi/pdf/10.1145/152610.152611)
- Une autre manière est d'exécuter chaque opération sur toute l'entrée
- La dernière est l'exécution par vecteurs, entre les deux.

* ### Calcul Vectoriel
- Plutôt que de charger un unique tuple, on charge un vecteur d'environ la taille du cache L1 du CPU
- Par exemple, on filtre en triant avec un vecteur de colonne 
- Beaucoup d'avantages décrit dans le Doc
[Comparatif et analyse ici](https://ir.cwi.nl/pub/13807/13807B.pdf)

* ### Compression
- Ranger les données par colonnes permet bien plus souvent de compresser.  
  Il existe bien des manières de compresser, et elles varient selon le type de données.  
  Ex: On ne compresse pas de la même manière des numéros de téléphone que des noms de famille
  * RLE
  * Bit-Vector Encoding
  * Dictionary
  * Frame Of Reference 

* ### Agir directement sur des données compressées

Il est possible d'exécuter des fonctions sur des données sans les décompresser (Gain de Cycles CPU).
Par exemple sommer des nombres. Se traite encore une fois au cas par cas

* ### [Late Materialization](https://ceur-ws.org/Vol-3130/paper3.pdf)

Les informations liées à une personne sont stockées côte-à-côte dans une RBDB ce qui est pratique car la plupart des requêtes accèdent à plusieurs champs d'un coup.  
À l'inverse, c'est moins optimisé quand on travaille par colonne. 
Une méthode pourrait être de regrouper les données par tuples dès les premières opérations (prédicats / projections) mais en pratique, on préfère appliquer ces opérations sur les colonnes en regroupant le plus tard possible.  
Schéma très parlant p50

* ### Jointures 

Les méthodes traditionnelles peuvent fonctionner, mais on peut trouver plus optimal. L'article prend l'exemple du "Jive" joins.

* ### Opérations UPDATE, DELETE, INSERT

Moins optimisées, car il faut effectuer N lectures/écritures avec N le nombre d'attributs de 
la table

* ### Indexation et "Cracking" 

Même si les opérations dans les BDD Colonnes se résument à une boucle 
scannant une colonne (ce qui dépasse déjà la perf d'une BDD Lignes) l'implémentation d'index 
peut tout de même accélérer grandement le processus.  
Ex: Regrouper les infos par rapport à un pivot pour faciliter la recherche





