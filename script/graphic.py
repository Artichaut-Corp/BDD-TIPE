import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Fichier CSV
CSV_FILE = "../script/data.csv"  # <-- mets le vrai chemin

# Paramètres à considérer pour le regroupement
GROUP_COLS = [
    "SelectionDescent",
    "PronfMode",
    "InsertProj",
    "OptimizeBinaryExpression",
    "OrderingQueryJoin"
]

# Taille du batch de requêtes pour calculer la moyenne
BATCH_SIZE = 100

# Lire le CSV
df = pd.read_csv(CSV_FILE, sep=";")

# Vérifier que les colonnes existent
missing = [col for col in GROUP_COLS + ["temps"] if col not in df.columns]
if missing:
    raise ValueError(f"Colonnes manquantes dans le CSV: {missing}")

# Regrouper par combinaison des paramètres
grouped = df.groupby(GROUP_COLS)

# Préparer le graphique
plt.figure(figsize=(12, 6))

for name, group in grouped:
    # Trier pour garder l'ordre original
    group = group.reset_index(drop=True)
    
    # Extraire les temps
    temps_list = group['temps'].values
    
    x_vals = []
    y_vals = []
    
    # Parcourir le groupe par batch de BATCH_SIZE
    for i in range(0, len(temps_list), BATCH_SIZE):
        chunk = temps_list[i:i+BATCH_SIZE]
        x_vals.append(i + len(chunk)//2 + 1)  # position centrale du batch
        y_vals.append(np.mean(chunk))
    
    # Créer un label en fonction des paramètres
    label = ",".join(str(v) for v in name)
    
    # Tracer la courbe
    plt.plot(x_vals, y_vals, marker='o', label=label)
    print("fini")

plt.xlabel("Nombre de requêtes (position batch)")
plt.ylabel("Temps moyen (microsecondes)")
plt.title(f"Temps moyen par batch de {BATCH_SIZE} requêtes selon les paramètres")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
