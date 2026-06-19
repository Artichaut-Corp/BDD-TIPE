import csv
import re

sql_file = "/home/eliott/fichier/WikipediaDump/Fr/frwiki-20251101-category.sql"
csv_file = "script/table/category.csv"

column_names = ["cat_id", "cat_title", "cat_pages", "cat_subcats", "cat_files"]

tuple_pattern = re.compile(r"\((.*?)\)", re.DOTALL)

field_pattern = re.compile(
    r"""
    '([^'\\]*(?:\\.[^'\\]*)*)' | 
    ([^,]+)                      
    """,
    re.VERBOSE,
)

def decode_sql_string(s):
    return (
        s.replace("\\\\", "\\")
         .replace("\\'", "'")
    )

with open(sql_file, "r", encoding="utf8") as f_sql, \
     open(csv_file, "w", newline="", encoding="utf8") as f_csv:
    writer = csv.writer(f_csv)

    # 🔥 AJOUT DE LA LIGNE D'EN-TÊTE
    writer.writerow(column_names)

    for line in f_sql:
        if "INSERT INTO" not in line:
            continue

        tuples = tuple_pattern.findall(line)
        for t in tuples:

            fields = []
            for q, u in field_pattern.findall(t):
                if q:  # quoted
                    fields.append(decode_sql_string(q))
                else:
                    val = u.strip()
                    if val == "NULL":
                        fields.append("")
                    else:
                        fields.append(val)

            writer.writerow(fields)

print("CSV created:", csv_file)
