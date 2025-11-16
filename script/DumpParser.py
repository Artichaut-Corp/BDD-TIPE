import os
import csv
import mwxml
import time
from datetime import datetime

# -----------------------------
# Configuration
# -----------------------------
dump_path = "/home/eliott/fichier/WikipediaDump/Fr/frwiktionary-20251101-pages-articles-multistream.xml/frwiktionary-20251101-pages-articles-multistream.xml"
table_dir = "script/table"
os.makedirs(table_dir, exist_ok=True)

# -----------------------------
# CSV setup (sans dbname ni siteinfo)
# -----------------------------
csv_files = {
    "page.csv": ["id", "ns", "title", "revision_id"],
    "revision.csv": ["id", "parentid", "timestamp", "contributor_id"],
    "contributor.csv": ["id", "username"],
    "namespaces.csv": ["key", "name"]
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
# Nombre total de pages (déjà connu)
# -----------------------------
total_pages = 7502789  # pour le dump frwiktionary 20251101

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
        writers["namespaces.csv"].writerow([
            getattr(ns, "key", ""),
            getattr(ns, "name", "")
        ])

    # -------------------------
    # Pages, Revisions, Contributors
    # -------------------------
    pages_processed = 0
    for page in dump.pages:
        if page.redirect is not None:
            continue
        latest_revision_id = ""
        revisions = []
        for rev in page:
            revisions.append(rev)
            latest_revision_id = rev.id

        # Écriture page.csv
        writers["page.csv"].writerow([
            page.id,
            page.namespace,
            page.title,
            latest_revision_id
        ])

        # Écriture revision.csv + contributor.csv
        for rev in revisions:
            user = getattr(rev, "user", None)
            user_id = getattr(user, "id", 0) if user else 0
            user_name = getattr(user, "text", "") if user else ""
            if user_id == None:
                user_id = 0
            writers["revision.csv"].writerow([
                rev.id,
                rev.parent_id if rev.parent_id else "",
                timestamp_to_int(rev.timestamp),
                user_id
            ])
            if timestamp_to_int(rev.timestamp) == 1748727692:
                print(user_id)

            if user and user_id not in contributors_seen:
                writers["contributor.csv"].writerow([user_id, user_name])
                contributors_seen.add(user_id)

        # -------------------------
        # Progression
        # -------------------------
        pages_processed += 1
        if pages_processed % 1000 == 0:
            percent = (pages_processed / total_pages) * 100
            print(f"\rProgress: {pages_processed}/{total_pages} pages ({percent:.2f}%)", end="")
            if percent>50:
                break

print("\n✅ CSV files created successfully in the TABLE directory.")

# -----------------------------
# Fermeture des fichiers
# -----------------------------
for f in files.values():
    f.close()
