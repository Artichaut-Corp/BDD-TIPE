import py_bddtipe
from random import randint
pathdb = "./build/main.db"
csv= "script/data.csv"
queryfilepath = "script/test.txt"
e = py_bddtipe.DatabaseEngine()

def run(n, s):
    e.Init(s)
    fp = open(queryfilepath)
    lines = fp.readlines()
    for i in range(n):
        line_pos = randint(0,len(lines))
        result = e.Exec(lines[line_pos][:-1])
        if result != "SELECT SUCESS":
            print("Avec La Querry : " + lines[line_pos][:-1]+ "\n")
            print("Erreur:" + result +"\n")
        
    
def Benchmark(n):
    print("=== SelectionDescent ===")
    
    run(n, py_bddtipe.DatabaseSetting(pathdb,1, 1, 0, 0, 0,1))
    print("=== PronfMode = 1 ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,0, 1, 0, 0, 0,1))

    print("=== PronfMode = 3 ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,0, 3, 0, 0, 0,1))

    print("=== InsertProj ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,0, 1, 1, 0, 0,1))

    print("=== OptimizeBinaryExpression ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,0, 1, 0, 1, 0,1))

    print("=== OrderingQueryJoin ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,0, 1, 0, 0, 1,1))

    print("=== FULL OPTIMIZATION ===")
    run(n,    py_bddtipe.DatabaseSetting(pathdb,1, 3, 1, 1, 1,1))

    print("🎉 Benchmark completed.")
    
if __name__ == "__main__":
    Benchmark(100)