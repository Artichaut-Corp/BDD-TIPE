```sql
TRANSACTION country (name, pop) VALUES("France", 67000000),("Allemagne", 80000000),("Japon", 120000000),("Corée du Sud", 50000000),("Norvège", 5000000),("Danemark", 5000000),("Suède", 10000000)END;

TRANSACTION country (name, pop) VALUES ("France", 67000000),("Allemagne", 80000000), ("Japon", 120000000), ("Corée du Sud", 50000000), ("Norvège", 5000000), ("Danemark", 5000000), ("Suède", 10000000) END;

TRANSACTION city (country, pop, name)  VALUES ("France", 12000000, "Paris"), ("Allemagne", 4500000, "Berlin"), ("Japon", 40000000, "Tokyo"), ("Corée du Sud", 27000000, "Séoul"), ("Norvège", 1200000, "Oslo"), ("Danemark", 1300000, "Copenhague"), ("Suède", 2135000, "Stockholm") END;

TRANSACTION city (name, pop, country) VALUES ("Paris", 12000000, "France"), ("Berlin", 4500000, "Allemagne"), ("Tokyo", 40000000, "Japon") END;


Select city.country, city.name, city.pop from city where city.country = "France";

Select country.name, country.pop, city.country, city.name, city.pop from city Join country on country.name = city.country;

Select country.name, country.pop, city.country, city.name, city.pop from city Join country on country.name = city.country where country.name = "France";
```
