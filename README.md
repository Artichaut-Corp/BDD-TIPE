# BDD-TIPE 

Projet de base de donnée

Structure du projet
-------------------

    ├─ src/             Code source de l'application 
    |  ├─ algebrizer/   Code contenant la structure d'exectution et de traitement des requêtes après le parser
    |  ├─ data_process_system/    Contient la structure des données utilisé par l'algebrizer
    │  ├─ parser/         
    │  ├─ lexer/          
    │  ├─ storage/              
    │  ├─ lexer/
    │  ├─ operation/
    │  └─ utils/
    ├─  test/           Tests
    └─  lib/            Dépendences Extérieures
    └─ script/          Contient l'ensemble des script utile pour traiter les dumps de wikipedia, la conversion se fait xml->csv par la bibliothèque de wikipedia, et ensuite C++ traite ces csv pour les ajouter dans notre fichier main.db. Enfin Benchmark 1.0 puis Benchmark 2.0 contient un ensemble d'outils (d'abord de manière très laborieuse puis à l'aide d'une API créer pour ces besoins) pour tester sur beaucoup de donnée et de query différente le SGBD
    └─ res /            Contient les requête à éxécuter pour créer un petit jeux de donnée pour tester les query et les optimisations
    └─ pres /           Contient les diaporama et le code de ceux-ci qui ont été présenté aux professeur validant notre projet



Installation
-----
Il faut télécharger et démarrer le projet : 
    1) créer un dossier build
    2) lancer "cmake ..  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug"
    3) lancer "make all"

Pour le lancer, il suffit d'exécuter  ./src/bdd_tipe dans le dossier build

Une fois lancé, vous aurez accès la REPL, un fichier main.db sera créé, il est propre à notre projet et inutilisable par tout autre SGBD.

Vous devrez insérer les donnée à partir de la REPL, pour cela, veuillez lancer les transactions présente dans le fichier res/requests.sql .

Des exemples de requête que l'on traite actuellement sont présentés dans ce même fichier res/requests.sql

Si vous voulez sauvegarder les insertions des valeurs dans le fichier, il faut quitter la REPL en appuyant sur entrée après la fin de votre test.

TODO
----

- [x] Parser -> réécrire toutes les structures: plutôt Ok, il reste des constructeurs et des champs à implémenter selon 
[la documentation sqlite](https://www.sqlite.org/lang_keywords.html). Réalisé en janvier 2025
- [x] Parser -> créer les méthode et implémenter la fonction Parser::Parse(). Réalisé en février 2025
- [x] BDD -> Implémentation des opérations de sélection, projection, jointures et fonctions agrégatives
- [x] BDD -> Créer les plans de requête 
- [x] BDD -> Comment organiser les données dans le fichier contenant la BDD (csv / json / vraie solution) 
- [x] Interface -> Présenter et recevoir les données
- [X] Présentation -> Trouver un jeu de données adapté et des opérations dessus optimisables par notre algorithme


Traitement d'une requête SQL depuis la REPL jusqu'au résultat
----------

Tout d'abord la requête SQL passe par le lexer, il vérifie la syntaxe globale de la requête, et transforme les éléments clef en token.

Ensuite le Parser regroupe les tokens en différent objet : la partie entre le select et le 'from', les différents 'join', sur quelle colonne se fait le Group by, etc

Ces objets sont transmis à L'Algebrizer.

Celui-ci reprend les différentes parties pour extraire les éléments utiles, par exemple, il liste les colonnes utilisées et qui seront donc à charger, ou encore créer les objets qui appliqueront le group-by.

Il crée ensuite un premier plan, très naïf en fonction de l'ordre des 'join' qui lui ont été transmis, il crée ensuite les tables et les charges dans la RAM.

En fonction des paramètres fournie dans le fichier script/Parametre.toml, différentes heuristiques et optimisations du plan vont s'exécuté, chacune affichera les modifications appliquées sur le plan.

Le plan est ensuite exécuté par un parcours en profondeur détaillé dans src/data_process_system/explication.txt

Ensuite, s'il y a des opérations d'agrégation, une partie spécialement conçue pour ce cas s'exécuté, elle applique (s'il existe) le group by, via une MAP à plusieurs clefs adapté de la bibliothèque RobinHoodMap, ensuite applique les order By et enfin les 'limit'.

Ensuite, l'affichage de la table obtenue à la fin s'exécute.



Plus d'information
------
Pour plus de détail, une explication des différents paramètre est fourni dans Explication-Paramètre/md

Une explication de la gestion mémoire des tables dans le cadre d'une requête SQL de type select est décrite dans /src/data_process_system/explication.txt

Une liste d'erreur trouvée et d'optimisation prévue est décrite dans ToDo.txt

Nos notes de première lecture sur certain papier décrivant les SGBD colonne sont présentes dans notes.md







