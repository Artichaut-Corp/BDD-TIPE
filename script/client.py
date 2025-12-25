import py_bddtipe

s = py_bddtipe.DatabaseSetting()
e = py_bddtipe.DatabaseEngine()


e.Init(s)
print(e.Exec("SELECT name FROM country ORDER BY idh;"))

