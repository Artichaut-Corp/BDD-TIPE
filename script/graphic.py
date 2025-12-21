import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

OUTPUT_DIR = "./pres/eliott/ressource/"
os.makedirs(OUTPUT_DIR, exist_ok=True)
colors = plt.cm.tab10.colors
courbes = []  # pour stocker toutes les courbes (x, y)
# Fichier CSV
CSV_FILE = "script/data.csv"  # <-- mets le vrai chemin

# Paramètres à considérer pour le regroupement
GROUP_COLS = [
    "SelectionDescent",
    "PronfMode",
    "InsertProj",
    "OptimizeBinaryExpression",
    "OrderingQueryJoin"
]
labels = [
    "Jointure triée seulement",
    "Ordre de jointure",
    "Optimisant les conditions",
    "En insérant des projections",
    "Jointure par dictionnaire",
    "En descendant les sélections",
    "En appliquant toutes les optimisations"
]
# Taille du batch de requêtes pour calculer la moyenne
BATCH_SIZE = 100# Lire le CSV
df = pd.read_csv(CSV_FILE, sep=";")

# Vérifier que les colonnes existent
missing = [col for col in GROUP_COLS + ["temps"] if col not in df.columns]
if missing:
    raise ValueError(f"Colonnes manquantes dans le CSV: {missing}")

# Regrouper par combinaison des paramètres
grouped = df.groupby(GROUP_COLS)

# Préparer le graphique
plt.figure(figsize=(12, 6))
j = 0
for name, group in grouped:
    group = group.reset_index(drop=True)
    temps_list = group['temps'].values

    x_vals = []
    y_vals = []

    for i in range(0, len(temps_list), BATCH_SIZE):
        chunk = temps_list[i:i + BATCH_SIZE]
        x_vals.append(i + len(chunk)//2 + 1)
        y_vals.append(np.mean(chunk) / 1_000_000)

    courbes.append((x_vals, y_vals))

    plt.figure(figsize=(12, 6))

    # Première courbe (toujours bleue)
    plt.plot(
        courbes[0][0],
        courbes[0][1],
        marker='o',
        color='blue',
        label=labels[0]
    )

    # Courbe courante (couleur différente)
    if j != 0:
        plt.plot(
            x_vals,
            y_vals,
            marker='o',
            color=colors[j % len(colors)],
            label=labels[j]
        )

    plt.xlabel("Nombre de requêtes")
    plt.ylabel("Temps moyen (secondes)")
    plt.title(f"Comparaison : {labels[0]} vs {labels[j]}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    filename = f"{OUTPUT_DIR}/comparaison_0_vs_{j}.png"
    plt.savefig(filename, dpi=300)
    plt.close()

    print(f"Image sauvegardée : {filename}")
    j += 1
plt.figure(figsize=(12, 6))

for i, (x, y) in enumerate(courbes):
    if i == 0:
        plt.plot(
            x, y,
            marker='o',
            color='blue',
            linewidth=2.5,
            label=labels[i]
        )
    else:
        plt.plot(
            x, y,
            marker='o',
            color=colors[i % len(colors)],
            label=labels[i]
        )

plt.xlabel("Nombre de requêtes")
plt.ylabel("Temps moyen (secondes)")
plt.title("Comparaison de toutes les optimisations")
plt.legend()
plt.grid(True)
plt.tight_layout()

filename = f"{OUTPUT_DIR}/toutes_les_courbes.png"
plt.savefig(filename, dpi=300)
plt.show()
