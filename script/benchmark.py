#!/usr/bin/env python3
"""
Benchmark runner pour bdd-tipe REPL.

- Démarrage REPL avec 2 secondes d'attente.
- Détection du succès de chaque query.
- Attente de 0.5 seconde après la réussite avant d'envoyer la prochaine query.
"""

import subprocess
import time
import random
import os
import unicodedata

# ========================
# CONFIGURATION
# ========================
REPL_PATH = "../build/src/bdd_tipe"
PARAM_FILE = "../../../bdd-tipe/Parametre.toml"

CSV_PATH = "../script/data.csv"
NB_QUERY_PER_PARAM = 10000
os.makedirs("script", exist_ok=True)

csv_header = [
    "SelectionDescent",
    "PronfMode",
    "InsertProj",
    "OptimizeBinaryExpression", 
    "OrderingQueryJoin",
    "temps",
    "nbr_join",
]

# Overwrite file with header
with open(CSV_PATH, "w") as f:
    f.write(";".join(csv_header) + "\n")
QUERIES = [
    'Select city.country, city.name, city.pop from city where city.country = "France";',
    'Select country.name, country.pop, city.country, city.name, city.pop from city Join country on country.name = city.country;',
    'Select country.name, country.pop, city.country, city.name, city.pop from city Join country on country.name = city.country where country.name = "France";',
    'Select country.name, city.pop, president.first_name from city Join country on country.name = city.country JOIN president ON president.country = city.country;',
    'SELECT president.first_name, president.last_name, city.pop FROM country  JOIN president ON  president.country = country.name JOIN city ON city.country = president.country ;',
    'SELECT president.first_name FROM president JOIN country on  president.country = country.name  WHERE ( country.name = "France" AND country.pop > 5000000 );',
    'SELECT president.first_name FROM president JOIN country on country.name = president.country WHERE country.name = "France";',
    'SELECT first_name,last_name,Sum(city.pop) FROM president JOIN country ON country.name = president.country JOIN city ON city.country = president.country group by country.name order by first_name Limit 5 offset 5;',
    'SELECT first_name AS prenom_du_president, last_name AS nom_du_president, SUM(city.pop)  From president AS dirigeant JOIN country AS pays ON pays.name = dirigeant.country JOIN city AS ville ON ville.country = dirigeant.country GROUP BY pays.name, first_name, last_name ORDER BY first_name;',
    'SELECT first_name As truc ,last_name,Sum(city.pop) FROM president JOIN country ON country.name = president.country JOIN city ON city.country = president.country group by president.first_name,president.last_name,country.name order by first_name ;',
    'Select country, AVG(pop) from city group by country order by pop ASC Limit 10;',
    'Select MAX(city.pop) from city group by city.country ;',
    'Select count(city.name),city.country from city group by city.country ;',
    'Select count(city.name),city.country from city group by city.country order by city.country ASC ;',
    'SELECT name,pop FROM country WHERE pop > 100000000;',
    'SELECT city.name,city.pop FROM city WHERE country = "Italie";',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 10000000) AND (name = "France") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Chine") AND (pop > 1000000) ) ORDER BY pop DESC LIMIT 5;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Inde") AND (mandate_beginning > 1400000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 50000000) );',
    'SELECT SUM(city.pop) FROM city WHERE ( (country = "France") AND (pop > 1000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Inde") AND (city.pop > 500000) );',
    'SELECT country.name,country.pop FROM country ORDER BY pop DESC LIMIT 10 OFFSET 5;',
    'SELECT city.name FROM city WHERE ( (pop > 1000000) AND (country = "Japon") ) ORDER BY pop ASC;',
    'SELECT president.first_name FROM president WHERE ( (country = "Bresil") AND (mandate_beginning > 1600000000) );',
    'SELECT city.name,city.pop FROM city JOIN country ON city.country = country.name WHERE ( (country.pop < 10000000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 5000000) AND (name = "Allemagne") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Espagne") AND (pop < 10000000) ) ORDER BY pop DESC LIMIT 3;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Chine") AND (mandate_beginning > 1360000000) );',
    'SELECT city.name,city.pop FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 30000000) );',
    'SELECT MAX(city.pop) FROM city WHERE ( (country = "France") AND (pop > 2000000) );',
    'SELECT MIN(city.pop) FROM city WHERE ( (country = "Russie") AND (pop < 10000000) );',
    'SELECT country.name FROM country ORDER BY pop ASC LIMIT 5;',
    'SELECT city.name,city.pop FROM city WHERE ( (pop > 500000) AND (country = "Italie") );',
    'SELECT president.first_name FROM president WHERE ( (country = "Japon") AND (mandate_beginning > 1630000000) );',
    'SELECT city.name FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 20000000) );',
    'SELECT COUNT(city.name) FROM city WHERE ( (country = "Allemagne") AND (pop > 1000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Indonesie") AND (city.pop > 200000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 20000000) AND (name = "Espagne") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Bresil") AND (pop > 1000000) ) ORDER BY pop ASC LIMIT 10;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Inde") AND (mandate_beginning > 1400000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop < 5000000) );',
    'SELECT SUM(city.pop) FROM city WHERE ( (country = "Russie") AND (pop > 2000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Australie") AND (city.pop < 5000000) );',
    'SELECT country.name FROM country ORDER BY pop DESC LIMIT 7 OFFSET 2;',
    'SELECT city.name,city.pop FROM city WHERE ( (pop > 1000000) AND (country = "Nigeria") );',
    'SELECT president.first_name FROM president WHERE ( (country = "Bresil") AND (mandate_beginning > 1670000000) );',
    'SELECT city.name FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 10000000) );',
    'SELECT MAX(city.pop) FROM city WHERE ( (country = "Italie") AND (pop > 1000000) );',
    'SELECT MIN(city.pop) FROM city WHERE ( (country = "Espagne") AND (pop < 10000000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 50000000) AND (name = "Chine") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Japon") AND (pop > 5000000) ) ORDER BY pop DESC LIMIT 5;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "France") AND (mandate_beginning > 1490000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 40000000) );',
    'SELECT COUNT(city.name) FROM city WHERE ( (country = "Inde") AND (pop > 2000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Chine") AND (city.pop > 5000000) );',
    'SELECT country.name,pop FROM country ORDER BY pop ASC LIMIT 5 OFFSET 1;',
    'SELECT city.name,city.pop FROM city WHERE ( (pop > 1000000) AND (country = "Allemagne") );',
    'SELECT president.first_name FROM president WHERE ( (country = "Japon") AND (mandate_beginning > 1630000000) );',
    'SELECT city.name FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 30000000) );',
    'SELECT SUM(city.pop) FROM city WHERE ( (country = "France") AND (pop > 5000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Inde") AND (city.pop > 1000000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 20000000) AND (name = "Bresil") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Espagne") AND (pop < 10000000) ) ORDER BY pop DESC LIMIT 8;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Chine") AND (mandate_beginning > 1360000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 10000000) );',
    'SELECT MAX(city.pop) FROM city WHERE ( (country = "Russie") AND (pop > 5000000) );',
    'SELECT MIN(city.pop) FROM city WHERE ( (country = "Australie") AND (pop < 10000000) );',
    'SELECT country.name FROM country ORDER BY pop DESC LIMIT 10;',
    'SELECT city.name,city.pop FROM city WHERE ( (pop > 500000) AND (country = "Nigeria") );',
    'SELECT president.first_name FROM president WHERE ( (country = "Bresil") AND (mandate_beginning > 1670000000) );',
    'SELECT city.name FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 20000000) );',
    'SELECT COUNT(city.name) FROM city WHERE ( (country = "Italie") AND (pop > 1000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Espagne") AND (city.pop < 5000000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 10000000) AND (name = "France") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Chine") AND (pop > 1000000) ) ORDER BY pop ASC LIMIT 10;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Inde") AND (mandate_beginning > 1400000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop < 50000000) );',
    'SELECT SUM(city.pop) FROM city WHERE ( (country = "Japon") AND (pop > 1000000) );',
    'SELECT AVG(city.pop) FROM city JOIN country ON city.country = country.name WHERE ( (country.name = "Russie") AND (city.pop > 5000000) );',
    'SELECT country.name FROM country ORDER BY pop ASC LIMIT 7 OFFSET 3;',
    'SELECT city.name,city.pop FROM city WHERE ( (pop > 500000) AND (country = "Italie") );',
    'SELECT president.first_name FROM president WHERE ( (country = "France") AND (mandate_beginning > 1490000000) );',
    'SELECT city.name FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 40000000) );',
    'SELECT MAX(city.pop) FROM city WHERE ( (country = "Inde") AND (pop > 2000000) );',
    'SELECT MIN(city.pop) FROM city WHERE ( (country = "Chine") AND (pop < 20000000) );',
    'SELECT country.name,country.pop FROM country WHERE ( (pop > 50000000) AND (name = "Japon") );',
    'SELECT city.name,city.pop FROM city WHERE ( (country = "Bresil") AND (pop > 1000000) ) ORDER BY pop ASC LIMIT 5;',
    'SELECT president.first_name,president.last_name FROM president WHERE ( (country = "Russie") AND (mandate_beginning > 1330000000) );',
    'SELECT city.name,city.pop,city.country FROM city JOIN country ON city.country = country.name WHERE ( (country.pop > 20000000) );'
]
    

SUCCESS_TEXT = "Requête parfaitement executée"

# ========================
# NORMALISATION
# ========================
def norm(s: str) -> str:
    if s is None:
        return ""
    s = unicodedata.normalize("NFKD", s)
    s = "".join(c for c in s if not unicodedata.combining(c))
    s = s.lower()
    s = " ".join(s.split())
    return s

# ========================
# ÉCRITURE PARAMÈTRES
# ========================
def write_params(sel, pmode, iproj, optbin, ordjoin):
    os.makedirs(os.path.dirname(PARAM_FILE), exist_ok=True)
    with open(PARAM_FILE, "w", encoding="utf-8") as f:
        f.write(f"SelectionDescent = {int(bool(sel))}\n")
        f.write(f"PronfMode = {int(pmode)}\n")
        f.write(f"InsertProj = {int(bool(iproj))}\n")
        f.write(f"OptimizeBinaryExpression = {int(bool(optbin))}\n")
        f.write(f"OrderingQueryJoin = {int(bool(ordjoin))}\n")
        f.write("Benchmarking = 1\n")
    print(f"[INFO] Wrote parameters to {PARAM_FILE}")

# ========================
# LECTURE JUSQU'AU SUCCÈS
# ========================
def read_until_success(process, success_text=SUCCESS_TEXT, timeout=30.0):
    """Lit stdout ligne par ligne jusqu'à ce que le texte de succès apparaisse ou timeout."""
    output = ""
    deadline = time.time() + timeout
    trouvé = False
    while time.time() < deadline:
        line = process.stdout.readline()
        if not line:
            time.sleep(0.05)
            continue
        output += line
        if success_text in line:
            trouvé = True
            break
    return trouvé

# ========================
# DÉMARRER REPL
# ========================
def start_repl():
    if not os.path.exists(REPL_PATH):
        raise FileNotFoundError(f"REPL not found: {REPL_PATH}")

    process = subprocess.Popen(
        [REPL_PATH],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1  # line buffered
    )

    print("[INFO] Waiting 2 seconds for REPL to startup...")
    time.sleep(2)
    return process

# ========================
# EXECUTION DES QUERIES
# ========================
def run_queries(sel, pmode, iproj, optbin, ordjoin):
    print(f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} =====")
    write_params(sel, pmode, iproj, optbin, ordjoin)

    try:
        process = start_repl()
    except Exception as e:
        print("[FATAL] REPL startup failed:", e)
        return

    try:
        for i in range(NB_QUERY_PER_PARAM):
            query = random.choice(QUERIES)
            print(f"\n>>> Query {i+1}: {query}")

            # envoyer la query
            process.stdin.write(query + "\n")
            process.stdin.flush()

            # attendre succès
            out = read_until_success(process, SUCCESS_TEXT, timeout=15)
            time.sleep(0.5)  # attendre 0.5 seconde après succès

            if (not out):
                print(f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} =====")

                print(f"\n>>> Query {i+1}: {query}")

                print("❌ Query failed: success message not found")
                break

        print("🏁 Finished this configuration.")

    finally:
        try:
            if process.stdin:
                process.stdin.close()
        except:
            pass
        try:
            process.terminate()
            try:
                process.wait(timeout=1)
            except subprocess.TimeoutExpired:
                process.kill()
        except:
            pass

# ========================
# SEQUENCE COMPLETE DE BENCHMARK
# ========================
if __name__ == "__main__":
    random.seed(0)

    print("=== InsertProj ===")
    run_queries(0, 0, 1, 0, 0)


    print("=== FULL OPTIMIZATION ===")
    run_queries(1, 3, 1, 1, 1)

    print("🎉 Benchmark completed.")
