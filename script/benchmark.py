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
PARAM_FILE = "bdd-tipe/Parametre.toml"

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
    'SELECT contributors.username, pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "KhingArthur";',
    'SELECT contributors.username, revisions.timestamp, pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT contributors.username, pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Z484z";',
    'SELECT contributors.username, revisions.timestamp, pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT contributors.username, pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE contributors.username = "Robotinator1994" AND revisions.timestamp > 1388435335;',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "Renardeau.arctique";',
    'SELECT contributors.username, SUM(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username ORDER BY contributors.username LIMIT 5 OFFSET 5;',
    'SELECT contributors.username, SUM(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username ORDER BY contributors.username;',
    'SELECT contributors.username, SUM(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username ORDER BY contributors.username;',
    'SELECT contributors.username, AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username ORDER BY AVG(revisions.timestamp);',
    'SELECT MAX(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id GROUP BY contributors.username;',
    'SELECT contributors.username, COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username;',
    'SELECT contributors.username, COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id GROUP BY contributors.username ORDER BY contributors.username ASC;',
    'SELECT contributors.username FROM contributors;',
    'SELECT pages.title, revisions.timestamp FROM pages WHERE pages.title = "didine";',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "MPF";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1754975932 ORDER BY revisions.timestamp DESC LIMIT 5;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1761949678;',
    'SELECT pages.title, revisions.timestamp, contributors.username FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Z484z" AND revisions.timestamp > 1388435335;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Renardeau.arctique" AND revisions.timestamp > 1753511075;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username DESC LIMIT 10 OFFSET 5;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1388434111 ORDER BY revisions.timestamp ASC;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1762038459;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id;',
    'SELECT COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "MPF";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id ORDER BY revisions.timestamp DESC LIMIT 3;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1762035389;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id;',
    'SELECT MAX(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1754975932;',
    'SELECT MIN(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp < 1388433468;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username ASC LIMIT 5;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1753511075;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1762003866;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1761929470;',
    'SELECT COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1753511075;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Robotinator1994" AND revisions.timestamp > 1761949678;',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "Z484z";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1753511075 ORDER BY revisions.timestamp DESC LIMIT 8;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1360000000;',
    'SELECT pages.title, revisions.timestamp, contributors.username FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT MAX(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1761778384;',
    'SELECT MIN(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp < 1388433468;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username DESC LIMIT 10;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1753511075;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1670000000;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1761941381;',
    'SELECT COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1761941541;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Renardeau.arctique" AND revisions.timestamp > 1388434111;',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "MPF";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1753511075 ORDER BY revisions.timestamp DESC LIMIT 10;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1400000000;',
    'SELECT pages.title, revisions.timestamp, contributors.username FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT SUM(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Robotinator1994" AND revisions.timestamp > 500000;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username ASC LIMIT 7 OFFSET 3;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1490000000;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 40000000;',
    'SELECT MAX(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 2000000;',
    'SELECT MIN(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp < 1388435335;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username DESC LIMIT 10;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 500000;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1670000000;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 20000000;',
    'SELECT COUNT(pages.id) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Z484z" AND revisions.timestamp < 5000000;',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "Robotinator1994";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000 ORDER BY revisions.timestamp ASC LIMIT 10;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1400000000;',
    'SELECT pages.title, revisions.timestamp, contributors.username FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id;',
    'SELECT SUM(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000;',
    'SELECT AVG(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE contributors.username = "Renardeau.arctique" AND revisions.timestamp > 5000000;',
    'SELECT contributors.username FROM contributors ORDER BY contributors.username ASC LIMIT 7 OFFSET 3;',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 500000;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1490000000;',
    'SELECT pages.title FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 40000000;',
    'SELECT MAX(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 2000000;',
    'SELECT MIN(revisions.timestamp) FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp < 20000000;',
    'SELECT contributors.username FROM contributors WHERE contributors.username = "Z484z";',
    'SELECT pages.title, revisions.timestamp FROM pages JOIN revisions ON pages.revision_id = revisions.id WHERE revisions.timestamp > 1000000 ORDER BY revisions.timestamp ASC LIMIT 5;',
    'SELECT contributors.username FROM contributors JOIN revisions ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 1330000000;',
    'SELECT pages.title, revisions.timestamp, contributors.username FROM pages JOIN revisions ON pages.revision_id = revisions.id JOIN contributors ON revisions.contributor_id = contributors.id WHERE revisions.timestamp > 20000000;'
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

    print("=== BASELINE CONFIG ===")
    run_queries(0, 0, 0, 0, 0)

    print("=== SelectionDescent ===")
    run_queries(1, 0, 0, 0, 0)

    print("=== PronfMode = 1 ===")
    run_queries(0, 1, 0, 0, 0)

    print("=== PronfMode = 3 ===")
    run_queries(0, 3, 0, 0, 0)

    print("=== InsertProj ===")
    run_queries(0, 0, 1, 0, 0)

    print("=== OptimizeBinaryExpression ===")
    run_queries(0, 0, 0, 1, 0)

    print("=== OrderingQueryJoin ===")
    run_queries(0, 0, 0, 0, 1)

    print("=== FULL OPTIMIZATION ===")
    run_queries(1, 3, 1, 1, 1)

    print("🎉 Benchmark completed.")
