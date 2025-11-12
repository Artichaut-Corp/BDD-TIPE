import os
import csv
import mwxml
import time
from datetime import datetime

# -----------------------------
# Configuration
# -----------------------------
dump_path = "/home/eliott/fichier/WikipediaDump/Fr/frwiktionary-20251101-pages-articles-multistream.xml/frwiktionary-20251101-pages-articles-multistream.xml"
table_dir = "script/TABLE"
os.makedirs(table_dir, exist_ok=True)

# -----------------------------
# CSV setup (sans dbname ni siteinfo)
# -----------------------------
csv_files = {
    "page.csv": ["id", "ns", "title", "redirect", "revision_id"],
    "revision.csv": ["id", "parentid", "timestamp", "model", "format", "contributor_id"],
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
# # Count total pages (approximation) 
# ----------------------------- 
#print("Counting total pages (this may take a while)...") 
#with open(dump_path, "rb") as f: 
# dump_for_count = mwxml.Dump.from_file(f) 
# total_pages = 0 
# for _ in dump_for_count.pages: 
# total_pages +=1 
# if total_pages %100_000 == 0:
# print(total_pages) #print(f"Total pages: {total_pages}") 

total_pages = 7502789 #for the french dump of 20251101
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
            ns.id,
            ns.name
        ])

    # -------------------------
    # Pages, Revisions, Contributors
    # -------------------------
    pages_processed = 0
    for page in dump.pages:
        redirect_title = page.redirect.title if page.redirect else ""

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
            redirect_title,
            latest_revision_id
        ])

        # Écriture revision.csv + contributor.csv
        for rev in revisions:
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

        # -------------------------
        # Progression
        # -------------------------
        pages_processed += 1
        percent = (pages_processed / total_pages) * 100
        print(f"\rProgress: {pages_processed}/{total_pages} pages ({percent:.2f}%)", end="")

print("\n✅ CSV files created successfully in the TABLE directory.")

# -----------------------------
# Fermeture des fichiers
# -----------------------------
for f in files.values():
    f.close()
