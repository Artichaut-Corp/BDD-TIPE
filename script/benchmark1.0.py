
import os
import random
import subprocess
import time
import unicodedata

# ========================
# CONFIGURATION
# ========================
REPL_PATH = "./src/bdd_tipe"
PARAM_FILE = "/home/eliott/bdd-tipe/Parametre.toml"

CSV_PATH = "../script/data.csv"
NB_QUERY_PER_PARAM = 82
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
        bufsize=1,  # line buffered
    )

    print("[INFO] Waiting 2 seconds for REPL to startup...")
    time.sleep(2)
    return process


# ========================
# EXECUTION DES QUERIES
# ========================
def run_queries(sel, pmode, iproj, optbin, ordjoin):
    print(
        f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} ====="
    )
    write_params(sel, pmode, iproj, optbin, ordjoin)

    try:
        process = start_repl()
    except Exception as e:
        print("[FATAL] REPL startup failed:", e)
        return

    try:
        for i in range(NB_QUERY_PER_PARAM):
            query = QUERIES[i]
            print(f"\n>>> Query {i + 1}: {query}")

            # envoyer la query
            process.stdin.write(query + "\n")
            process.stdin.flush()

            # attendre succès
            out = read_until_success(process, SUCCESS_TEXT, timeout=15)
            time.sleep(0.5)  # attendre 0.5 seconde après succès

            if not out:
                print(
                    f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} ====="
                )

                print(f"\n>>> Query {i + 1}: {query}")

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


    print("=== SelectionDescent ===")
    run_queries(1, 1, 0, 0, 0)

    print("=== PronfMode = 1 ===")
    run_queries(0, 1, 0, 0, 0)

    print("=== PronfMode = 3 ===")
    run_queries(0, 3, 0, 0, 0)

    print("=== InsertProj ===")
    run_queries(0, 1, 1, 0, 0)

    print("=== OptimizeBinaryExpression ===")
    run_queries(0, 1, 0, 1, 0)

    print("=== OrderingQueryJoin ===")
    run_queries(0, 1, 0, 0, 1)

    print("=== FULL OPTIMIZATION ===")
    run_queries(1, 3, 1, 1, 1)

    print("🎉 Benchmark completed.")
