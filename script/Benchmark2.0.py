from random import randint

import py_bddtipe

pathdb = "./main.db"
csv = "../script/data.csv"
queryfilepath = "../script/test.txt"
e = py_bddtipe.DatabaseEngine()
#RUN FROM BUILD !!!!!
csv_header = [
    "SelectionDescent",
    "PronfMode",
    "InsertProj",
    "OptimizeBinaryExpression",
    "OrderingQueryJoin",
    "SampleSize"
    "DataSize",
    "temps",
    "nbr_join",
]
# Overwrite file with header
with open(csv, "w") as f:
    f.write(";".join(csv_header) + "\n")
    
def run(n, s):
    e.Init(s)
    fp = open(queryfilepath)
    lines = fp.readlines()
    for i in range(n):
        line_pos = randint(0, len(lines))
        print("Requête: " + lines[line_pos][:-1] + "\n")

        result = e.Exec(lines[line_pos][:-1])
        if result != "SELECT SUCESS":
            print("Erreur:" + result + "\n")
    e.InsertCsvData()


def Benchmark(n):
    for i in range(100):
        e.Init(py_bddtipe.DatabaseSetting(pathdb, 1, 1, 0, 0, 0, 1,min(i*100,1000),(i+1)*1000))
        e.InsertCsvData(i*1000,1000)
        print("=== SelectionDescent ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 1, 1, 0, 0, 0, 1,min(i*100,1000),(i+1)*1000))
        
        print("=== PronfMode = 1 ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 0, 1, 0, 0, 0, 1,min(i*100,1000),(i+1)*1000))

        print("=== PronfMode = 3 ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 0, 3, 0, 0, 0, 1,min(i*100,1000),(i+1)*1000))

        print("=== InsertProj ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 0, 1, 1, 0, 0, 1,min(i*100,1000),(i+1)*1000))

        print("=== OptimizeBinaryExpression ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 0, 1, 0, 1, 0, 1,min(i*100,1000),(i+1)*1000))

        print("=== OrderingQueryJoin ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 0, 1, 0, 0, 1, 1,min(i*100,1000),(i+1)*1000))

        print("=== FULL OPTIMIZATION ===")
        run(n, py_bddtipe.DatabaseSetting(pathdb, 1, 3, 1, 1, 1, 1,min(i*100,1000),(i+1)*1000))



if __name__ == "__main__":
    Benchmark(100)
