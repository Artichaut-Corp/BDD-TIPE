TRANSACTION
INSERT INTO pays
VALUES(1, 'France');
INSERT INTO pays
VALUES(2, 'Allemagne');
INSERT INTO pays
VALUES(3, 'Italie');
INSERT INTO pays
VALUES(4, 'Angleterre');
INSERT INTO pays
VALUES(5, 'Belgique');
INSERT INTO pays
VALUES(6, 'Pays-Bas');
INSERT INTO pays
VALUES(7, 'Espagne');
INSERT INTO pays
VALUES(8, 'États-Unis');
END;

TRANSACTION
INSERT INTO peintres
VALUES(10, 'Sanzio', 'Raffaello', 'Raphaël', 3, 1483, 1520);
INSERT INTO peintres
VALUES(
    20,
    'di ser Piero da Vinci',
    'Leonardo',
    'Léonard de Vinci',
    3,
    1452,
    1519
  );
INSERT INTO peintres
VALUES(
    30,
    'di Lodovico Buonarroti Simoni',
    'Michelangelo',
    'Michel-Ange',
    3,
    1475,
    1564
  );
INSERT INTO peintres
VALUES(40, 'Manet', 'Édouard', NULL, 1, 1832, 1883);
INSERT INTO peintres
VALUES(
    50,
    'di Mariano di Vanni Filipepi',
    'Alessandro',
    'Sandro Botticelli',
    3,
    1445,
    1510
  );
INSERT INTO peintres
VALUES(
    60,
    'Merisi da Caravaggio',
    'Michelangelo',
    'Le Caravage',
    3,
    1571,
    1610
  );
INSERT INTO peintres
VALUES(70, 'Grünewald', 'Matthias', NULL, 2, 1480, 1528);
INSERT INTO peintres
VALUES(80, 'Vecellio', 'Tiziano', 'Le Titien', 3, 1490, 1576);
INSERT INTO peintres
VALUES(90, 'van Eyck', 'Jan', NULL, 5, 1390, 1441);
INSERT INTO peintres
VALUES(100, 'Dürer', 'Albrecht', NULL, 2, 1471, 1528);
INSERT INTO peintres
VALUES(110, 'Rubens', 'Pieter Paul', NULL, 5, 1577, 1640);
INSERT INTO peintres
VALUES(
    120,
    'Watteau',
    'Jean-Antoine',
    'Watteau',
    1,
    1684,
    1721
  );
INSERT INTO peintres
VALUES(130, 'Vermeer', 'Johannes', NULL, 6, 1632, 1675);
INSERT INTO peintres
VALUES(140, 'Gainsborough', 'Thomas', NULL, 4, 1727, 1788);
INSERT INTO peintres
VALUES(150, 'David', 'Jacques-Louis', NULL, 1, 1748, 1825);
INSERT INTO peintres
VALUES(160, 'Géricault', 'Théodore', NULL, 1, 1791, 1824);
INSERT INTO peintres
VALUES(170, 'Monet', 'Claude', NULL, 1, 1840, 1926);
INSERT INTO peintres
VALUES(180, 'van Gogh', 'Vincent', NULL, 6, 1848, 1890);
INSERT INTO peintres
VALUES(190, 'Kandinsky', 'Vassily', NULL, 2, 1866, 1944);
INSERT INTO peintres
VALUES(200, 'Picasso', 'Pablo', NULL, 7, 1881, 1973);
INSERT INTO peintres
VALUES(210, 'Miró', 'Joan', NULL, 7, 1893, 1983);
INSERT INTO peintres
VALUES(
    220,
    'Harmenszoon van Rijn',
    'Rembrandt',
    'Rembrandt',
    6,
    1606,
    1669
  );
INSERT INTO peintres
VALUES(230, 'van der Weyden', 'Rogier', NULL, 5, 1399, 1464);
INSERT INTO peintres
VALUES(240, 'Stingel', 'Rudolf', NULL, 3, 1956, NULL);
INSERT INTO peintres
VALUES(241, 'Millais', 'John Everett', NULL, 4, 1829, 1896);
INSERT INTO peintres
VALUES(242, 'Waterhouse', 'John William', NULL, 4, 1849, 1917);
INSERT INTO peintres
VALUES(243, 'Collier', 'John', NULL, 4, 1850, 1934);
INSERT INTO peintres
VALUES(244, 'Goodridge', 'Sarah', NULL, 8, 1788, 1853);
END;

TRANSACTION
INSERT INTO villes
VALUES(1, 'Paris', 1);
INSERT INTO villes
VALUES(2, 'Londres', 4);
INSERT INTO villes
VALUES(3, 'Colmar', 1);
INSERT INTO villes
VALUES(4, 'Rome', 3);
INSERT INTO villes
VALUES(5, 'Berlin', 2);
INSERT INTO villes
VALUES(6, 'Dresde', 2);
INSERT INTO villes
VALUES(7, 'Amsterdam', 6);
INSERT INTO villes
VALUES(8, 'Coventry', 4);
INSERT INTO villes
VALUES(9, 'New-York', 8);
END;

TRANSACTION
INSERT INTO musees
VALUES(1, 'Musée du Louvre', 1);
INSERT INTO musees
VALUES(2, 'National Gallery', 2);
INSERT INTO musees
VALUES(3, 'Galerie Borghèse', 4);
INSERT INTO musees
VALUES(4, 'Gemäldegalerie', 5);
INSERT INTO musees
VALUES(5, 'Musée d''Orsay', 1);
INSERT INTO musees
VALUES(6, 'Musée Unterlinden', 3);
INSERT INTO musees
VALUES(7, 'Musées du Vatican', 4);
INSERT INTO musees
VALUES(8, 'Gemäldegalerie', 6);
INSERT INTO musees
VALUES(9, 'Rijksmuseum', 7);
INSERT INTO musees
VALUES(10, 'Tate Britain', 2);
INSERT INTO musees
VALUES(11, 'Herbert Art Gallery', 8);
INSERT INTO musees
VALUES(12, 'Metropolitan Museum of Art', 9);
END;

TRANSACTION
INSERT INTO oeuvres
VALUES(1, 'Amour sacré et Amour profane', 80, 3);
INSERT INTO oeuvres
VALUES(2, 'Autoportrait de Van Gogh', 180, 5);
INSERT INTO oeuvres
VALUES(
    3,
    'Chemin dans la Forêt de Fontainebleau',
    170,
    NULL
  );
INSERT INTO oeuvres
VALUES(4, 'Dans le jardin du docteur Gachet', 180, 5);
INSERT INTO oeuvres
VALUES(5, 'David avec la tête de Goliath', 60, 3);
INSERT INTO oeuvres
VALUES(6, 'Deux enfants', 180, 5);
INSERT INTO oeuvres
VALUES(
    7,
    'Fritillaires, couronne impériale dans un vase de cuivre',
    180,
    5
  );
INSERT INTO oeuvres
VALUES(8, 'Jardin de l''hôpital Saint-Paul', 180, 5);
INSERT INTO oeuvres
VALUES(9, 'Jeune homme avec un chapeau', 80, 2);
INSERT INTO oeuvres
VALUES(10, 'L''École d''Athènes', 10, 7);
INSERT INTO oeuvres
VALUES(11, 'L''Église d''Auvers-sur-Oise', 180, 5);
INSERT INTO oeuvres
VALUES(12, 'La Dame à la Licorne', 10, 3);
INSERT INTO oeuvres
VALUES(13, 'La Joconde', 20, 1);
INSERT INTO oeuvres
VALUES(14, 'La Madone aux chardonneret', 100, 4);
INSERT INTO oeuvres
VALUES(15, 'La Sainte Famille', 80, 2);
INSERT INTO oeuvres
VALUES(16, 'La Vierge au lapin', 40, 1);
INSERT INTO oeuvres
VALUES(17, 'La Vierge au lapin', 80, 1);
INSERT INTO oeuvres
VALUES(18, 'La Vierge aux rochers', 20, 1);
INSERT INTO oeuvres
VALUES(19, 'La laitière', 130, 9);
INSERT INTO oeuvres
VALUES(20, 'La ronde de nuit', 220, 9);
INSERT INTO oeuvres
VALUES(21, 'Le Doge Nicolo Marcello', 80, 7);
INSERT INTO oeuvres
VALUES(22, 'Le Déjeuner sur l''herbe', 40, 5);
INSERT INTO oeuvres
VALUES(23, 'Le Jugement dernier', 30, 7);
INSERT INTO oeuvres
VALUES(24, 'Le Printemps', 50, 1);
INSERT INTO oeuvres
VALUES(25, 'Le Pèlerinage à l''île de Cythère', 120, 1);
INSERT INTO oeuvres
VALUES(26, 'Le Radeau de La Méduse', 160, 1);
INSERT INTO oeuvres
VALUES(27, 'Le Restaurant de la Sirène à Asnières', 180, 5);
INSERT INTO oeuvres
VALUES(28, 'Les Époux Arnolfini', 90, 2);
INSERT INTO oeuvres
VALUES(29, 'Olympia', 40, 5);
INSERT INTO oeuvres
VALUES(30, 'Portrait de Clarisse Strozzi', 80, 4);
INSERT INTO oeuvres
VALUES(31, 'Portrait de Lavinia', 80, 8);
INSERT INTO oeuvres
VALUES(
    32,
    'Portrait du Dr Gachet avec branche de digitale',
    180,
    5
  );
INSERT INTO oeuvres
VALUES(33, 'Retable d''Issenheim', 70, 6);
INSERT INTO oeuvres
VALUES(34, 'Tête d''un homme barbu', 200, NULL);
INSERT INTO oeuvres
VALUES(35, 'Vierge à l''Enfant avec des saints', 80, 8);
INSERT INTO oeuvres
VALUES(36, 'Ophelia', 241, 10);
INSERT INTO oeuvres
VALUES(37, 'The Lady of Shalott', 242, 10);
INSERT INTO oeuvres
VALUES(38, 'Lady Godiva', 243, 11);
INSERT INTO oeuvres
VALUES(39, 'Beauty Revealed', 244, 12);
END
