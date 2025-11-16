# BDD-TIPE 

Projet de base de donnée

Structure du projet
-------------------

    ├─ src/             Code source de l'application 
    │  ├─ parser/         
    │  ├─ lexer/          
    │  ├─ storage/              
    │  ├─ lexer/
    │  ├─ operation/
    │  └─ utils/
    ├─  test/           Tests
    └─  lib/            Dépendences Extérieures
    └─ script/          Contient l'ensemble des script utile pour traiter les dumps de wikipedia, la conversion se fait xml->csv par la bibliothèque de wikipedia, et ensuite C++ traite ces csv pour les ajouter dans notre fichier main.db
    └─ res /            Contient les requête à éxécuter pour créer un petit jeux de donnée pour tester les query et les optimisations
    └─ pres /           Contient les diaporama et le code de ceux-ci qui ont été présenté aux professeur validant notre projet


Features
--------
 
- [x] Parser SQL (Il manque 'UPDATE' et 'INSERT')
- [ ] Optimisée pour la lecture
- [ ] Vector Processing
- [ ] Late Materialization
- [ ] Compression adaptée au type
- [ ] Hash


Installation
-----
il faut télécharger le projet, le compiler en lançant "make all" dans le dossier build

pour le lancer il suffit de d'éxécuter  ./src/TIPE-BDD_run dans le dossier build

une fois lancer, vous aurez accès la REPL, un fichier main.db seras créé, il est propre à notre projet et inutilisable par tout autre SGBD

vous devrez insérez les donnée à partir de la REPL, pour cela veuillez lancer les transaction présente dans le fichier res/sample.md, pour tester le SGBD avec de plus grande donnée, seul wikipedia est utilisable, et seulement la version que les fichiers dans le dossier script extraient à partir du dump fr

Des exemples de requête que l'on traite actuelement sont présenté dans ce même fichier res/sample.md

Si vous voulez sauvegardez l'insertions des valeurs dans le fichier, il faut quitte la REPL en appuyant sur entrée après la fin de vos test

TODO
----

- [x] Parser -> réécrire toutes les structures: plutôt Ok, il reste des constructeurs et des champs à implémenter selon 
[la documentation sqlite](https://www.sqlite.org/lang_keywords.html). Réalisé en Janvier 2025
- [x] Parser -> créer les méthode et implémenter la fonction Parser::Parse(). Réalisé en Février 2025
- [x] BDD -> Implémentation des opérations de sélection, projection, jointures et fonctions aggrégatives
- [x] BDD -> Créer les plans de requête 
- [ ] BDD -> Comment organiser les données dans le fichier contenant la BDD (csv / json / vraie solution) 
- [x] Interface -> Présenter et recevoir les données


Traitement d'une requête SQL depuis la REPL jusqu'au résultat
----------

Tout d'abord la requête sql passe par le lexer, il vérifie la syntaxe global de la requête, et transforme les élément clef en token.

Ensuite le Parser regroupe les token en différent objet : la partie entre le select et le from, les différent join, sur quelle colonne se fait le Group by ...

Ces objets sont transmis à L'Algebrizer,

celui-ci reprend les différentes parties pour extraire les élement utiles, par exemple il liste les colonne utilisé et qui seront donc à charger, ou encore créer les objet qui appliqueront le group-by.

Il créer ensuite un premier plan, très naïf en fonction de l'ordre de join qui lui ont été transmis, il créer ensuite les tables, et les charges dans la RAM.

En fonction des parametres fourni dans le fichier Parametre.toml, différente heuristique et optimisation du plan vont s'éxécuté, chacune afficheras les modifications appliqué sur le plan.

Le plan est ensuite éxécuté par un parcours en profondeur détaillé dans src/data_process_system/explication.txt

ensuite, si il y as des opération d'agrégation, une partie spécialement concu pour ce cas s'éxécute, elle applique (si il existe) le group by, via une map à plusieurs clef adapté de la bibliothèque RobinHoodMap, ensuite applique les order By et enfin les limit.

Ensuite, l'affichage de la table obtenue à la fin s'éxécute



Plus d'information
------
pour plus de détail, une explication des différent paramètre est fourni dans Explication-Paramètre/md

Une explication de la gestion mémoire des tables dans le cadre d'une requête sql de type select est décrite dans /src/data_process_system/explication.txt

une liste d'erreur trouvé et d'optimisation prévue est décrite dans ToDo.txt

nos note de première lecture sur certain papier décrivant les SGBD colonne sont présente dans notes.md






