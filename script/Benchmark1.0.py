
import os
import random
import subprocess
import time
import unicodedata

# ========================
# CONFIGURATION
# ========================

PATHDB = "./main.db"
CSV_PATH = "../script/data.csv"
QUERY_FILE_PATH = "../script/test.txt"

REPL_PATH = "./src/bdd_tipe"
PARAM_FILE = "../script/parametre.toml"
NB_QUERY_PER_PARAM = 100
BATCH_SIZE = 20
DATA_INSERT_SIZE = 4000
NBR_COMPARISON = 25
sizesample = 1000
#RUN FROM BUILD
os.makedirs("script", exist_ok=True)

csv_header = [
    "SelectionDescent",
    "PronfMode",
    "InsertProj",
    "OptimizeBinaryExpression",
    "OrderingQueryJoin",
    "SampleSize","DataSize",
    "temps",
    "nbr_join",
]

# Overwrite file with header
with open(CSV_PATH, "w") as f:
    f.write(";".join(csv_header) + "\n")
    

SUCCESS_TEXT = "SELECT SUCCESS"
INSERT_SUCCESS_TEXT = "INSERT SUCCESS"

def write_params(sel, pmode, iproj, optbin, ordjoin,sizesample,datasize):
    os.makedirs(os.path.dirname(PARAM_FILE), exist_ok=True)
    with open(PARAM_FILE, "w", encoding="utf-8") as f:
        f.write(f"SelectionDescent = {int(bool(sel))}\n")
        f.write(f"PronfMode = {int(pmode)}\n")
        f.write(f"InsertProj = {int(bool(iproj))}\n")
        f.write(f"OptimizeBinaryExpression = {int(bool(optbin))}\n")
        f.write(f"OrderingQueryJoin = {int(bool(ordjoin))}\n")
        f.write("Benchmarking = 1\n")
        f.write(f"SizeSample = {int(sizesample)}\n")
        f.write(f"DataSize = {int(datasize)}\n")
    print(f"[INFO] Wrote parameters to {PARAM_FILE}")

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
def run_queries(sel, pmode, iproj, optbin, ordjoin,loop_number):
    datasize = (loop_number+1)*DATA_INSERT_SIZE
    
    print(
        f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} sizesample={sizesample} datasize={datasize} ====="
    )
    write_params(sel, pmode, iproj, optbin, ordjoin,sizesample,datasize)

    fp = open(QUERY_FILE_PATH)
    lines = fp.readlines()
    try:
        for i in range(NB_QUERY_PER_PARAM):
            if(i % BATCH_SIZE==0):#kill the REPL to prevent memory overflow from memory leak
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
                try:
                    process = start_repl()
                except Exception as e:
                    print("[FATAL] REPL startup failed:", e)
                    return
            
            line_pos = random.randint(0, len(lines)-1)
            query =lines[line_pos][:-1]
            
            print(f"\n>>> Query {i + 1}: {query}")

            # envoyer la query
            process.stdin.write(query + "\n")
            process.stdin.flush()

            # attendre succès
            out = read_until_success(process, SUCCESS_TEXT, timeout=20)
            time.sleep(0.5)  # attendre 0.5 seconde après succès

            if not out:
                print(
                    f"\n===== Config sel={sel} pmode={pmode} iproj={iproj} optbin={optbin} ordjoin={ordjoin} sizesample={sizesample} datasize={datasize} ====="
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


def insert_data(i):
    try:
        process = start_repl()
    except Exception as e:
        print("[FATAL] REPL startup failed:", e)
        return
    
    query = f".insert_data_offset {int(DATA_INSERT_SIZE)} {int(i*DATA_INSERT_SIZE)}"
    print(f"\n>>> Insertion: {query}")

    # envoyer la query
    process.stdin.write(query + "\n")
    process.stdin.flush()
    # attendre succès
    out = read_until_success(process, INSERT_SUCCESS_TEXT, timeout=20)
    time.sleep(0.5)  # attendre 0.5 seconde après succès
    
    
    if not out:
        print(f"\n>>> Insertion: {query}")
        print("❌ INSERT failed: success message not found")
        return
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
    


if __name__ == "__main__":
    random.seed(0)
    for i in range(NBR_COMPARISON):
        print("=== Insertion des données ===")
        insert_data(i)
        
        print("=== SelectionDescent ===")
        run_queries(1, 1, 0, 0, 0,i)

        print("=== PronfMode = 1 ===")
        run_queries(0, 1, 0, 0, 0,i)

        print("=== PronfMode = 3 ===")
        run_queries(0, 3, 0, 0, 0,i)

        print("=== InsertProj ===")
        run_queries(0, 1, 1, 0, 0,i)

        print("=== OptimizeBinaryExpression ===")
        run_queries(0, 1, 0, 1, 0,i)

        print("=== OrderingQueryJoin ===")
        run_queries(0, 1, 0, 0, 1,i)

        print("=== FULL OPTIMIZATION ===")
        run_queries(1, 3, 1, 1, 1,i)

        print("🎉 Benchmark completed.")
