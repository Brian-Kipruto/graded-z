#!/usr/bin/env python3

'''this file sums the columns of a Geant4 CSV ntuple (pass 00 defect 15 in my obsidian notes).

Geant4 writes no plain header row. Column names live in the comment lines of the  form "#column <type><name>"; the data rows then follow.SO im gonna parse them 
from those lines instead of trusting pandas to find a header.

The usage will be in;
Python csvsum.py FILE.csv                         # sum + mean of every column
python csvsum.py FILE.csv col_a col_b             # only these columns
python csvsum.py FILE.csv --rows                  # header-line and data row counts

'''

import sys 
import csv

def read_g4csv(path):
    names, rows, n_comment = [], [], 0
    with open(path, newline="") as f:
        for line in f:
            if line.startswith("#"):
                n_comment +=1
                parts = line.split()
                if parts[0] == "#column" and len(parts) >= 3:
                    names.append(parts[2])
            elif line.strip():
                rows.append(next(csv.reader([line])))
    return names, rows, n_comment


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    names, rows, n_comment = read_g4csv(argv[1])
    if "--rows" in argv:
        print(f"comment lines: {n_comment}  columns: {len(names)}  data rows: {len(rows)}")
        return
    bad = [r for r in rows if len(r) != len(names)]
    if bad:
        sys.exit(f"{len(bad)} rows do not have {len(names)} fields")
    wanted = argv[2:] or names
    print(f"{'column':<26}{'sum':>16}{'mean/event':>14}   (N = {len(rows)})")
    for name in wanted:
        i = names.index(name)                      # ValueError = typo in name
        total = sum(float(r[i]) for r in rows)
        print(f"{name:<26}{total:>16.6g}{total / max(len(rows), 1):>14.6g}")


if __name__ == "__main__":
    main(sys.argv)

