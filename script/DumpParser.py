import os
import csv
import mwxml
import time
import re
from datetime import datetime

# -----------------------------
# Configuration
# -----------------------------
dump_path = "/home/eliott/fichier/WikipediaDump/Fr/frwiktionary-20251101-pages-articles-multistream.xml/frwiktionary-20251101-pages-articles-multistream.xml"
table_dir = "script/TABLE"
os.makedirs(table_dir, exist_ok=True)

# -----------------------------
# CSV setup
# -----------------------------
csv_files = {
    "page.csv": ["id", "ns", "title", "redirect", "revision_id"],
    "revision.csv": ["id", "parentid", "timestamp", "model", "format", "contributor_id"],
    "contributor.csv": ["id", "username"],
    "namespaces.csv": ["key", "name"],
    "categories_pages.csv": ["id_cat", "page_id"],
    "categories.csv": ["id", "name"]
}

files = {}
writers = {}
for filename, headers in csv_files.items():
    path = os.path.join(table_dir, filename)
    f = open(path, "w", newline="", encoding="utf-8")
    writer = csv.writer(f)
    writer.writerow(headers)
    files[filename] = f
    writers[filename] = writer

contributors_seen = set()
categories_seen = {}  # {category_name: category_id}
category_counter = 1  # auto-increment pour id de catégorie

# -----------------------------
# Helper function: convert mwxml.Timestamp to UNIX int
# -----------------------------
def timestamp_to_int(ts):
    try:
        dt = ts.asdatetime()
    except AttributeError:
        dt = datetime.strptime(str(ts), "%Y-%m-%dT%H:%M:%SZ")
    return int(time.mktime(dt.timetuple()))

# -----------------------------
# Helper function: extract categories from wikitext
# -----------------------------
category_pattern = re.compile(r"\[\[\s*Catégorie\s*:\s*([^\]\|#]+)", re.IGNORECASE)

def extract_categories(text):
    """Retourne une liste de catégories trouvées dans le texte wiki."""
    if not text:
        return []
    return [c.strip() for c in category_pattern.findall(text)]

# -----------------------------
# Total pages (estimé)
# -----------------------------
total_pages = 7502789  # pour le dump frwiktionary-20251101

# -----------------------------
# Parse the dump
# -----------------------------
with open(dump_path, "rb") as f:
    dump = mwxml.Dump.from_file(f)

    # -------------------------
    # Namespaces
    # -------------------------
    si = dump.site_info
    for ns in si.namespaces or []:
        writers["namespaces.csv"].writerow([ns.id, ns.name])

    # -------------------------
    # Pages, Revisions, Contributors, Categories
    # -------------------------
    pages_processed = 0

    for page in dump.pages:
        redirect_title = page.redirect.title if page.redirect else -1

        latest_revision = None
        for rev in page:
            latest_revision = rev  # on garde la dernière

        # --- page.csv ---
        writers["page.csv"].writerow([
            page.id,
            page.namespace,
            page.title,
            redirect_title,
            latest_revision.id if latest_revision else ""
        ])

        # --- revision.csv + contributor.csv ---
        for rev in page:
            user = getattr(rev, "user", None)
            user_id = getattr(user, "id", "") if user else ""
            user_name = getattr(user, "text", "") if user else ""

            writers["revision.csv"].writerow([
                rev.id,
                rev.parent_id if rev.parent_id else "",
                timestamp_to_int(rev.timestamp),
                rev.model if rev.model else "",
                rev.format if rev.format else "",
                user_id
            ])

            if user and user_id not in contributors_seen:
                writers["contributor.csv"].writerow([user_id, user_name])
                contributors_seen.add(user_id)

        # --- categories ---
        if latest_revision and latest_revision.text:
            cats = extract_categories(latest_revision.text)
            if page.title == "Amsterdam":
                print(latest_revision.text)
                print(cats)
            for cat in cats:
                # Ajouter à categories.csv si nouvelle
                if cat not in categories_seen:
                    categories_seen[cat] = category_counter
                    writers["categories.csv"].writerow([category_counter, cat])
                    category_counter += 1
                cat_id = categories_seen[cat]

                # Ajouter à la table de liaison
                writers["categories_pages.csv"].writerow([category_counter, page.id])

        # --- Progression ---
        pages_processed += 1
        if pages_processed % 1000 == 0:
            percent = (pages_processed / total_pages) * 100
            if percent > 5:
                break
            print(f"\rProgress: {pages_processed}/{total_pages} pages ({percent:.2f}%)", end="")

print("\n✅ CSV files created successfully in the TABLE directory.")

# -----------------------------
# Fermeture des fichiers
# -----------------------------
for f in files.values():
    f.close()
