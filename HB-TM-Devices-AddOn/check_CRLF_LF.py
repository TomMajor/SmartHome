
# Aufruf:
# python check_CRLF_LF.py CCU_RM

import argparse
from pathlib import Path

# Farben definieren
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
RESET = "\033[0m"

EXCLUDED_EXTENSIONS = {'.png', '.jpg', '.jpeg', '.gif', '.pdf', '.exe', '.zip', '.pyc', '.tgz'}

def check_line_endings(file_path):
    has_lf = False
    has_crlf = False
    
    try:
        with open(file_path, 'rb') as f:
            for line in f:
                if line.endswith(b'\r\n'):
                    has_crlf = True
                elif line.endswith(b'\n'):
                    has_lf = True
                
                if has_lf and has_crlf:
                    return f"{RED}MIXED{RESET}"
        
        if has_lf and has_crlf: return f"{RED}MIXED{RESET}"
        if has_crlf: return f"{RED}CRLF{RESET}" # Rot markiert
        if has_lf: return f"{GREEN}LF{RESET}"   # Grün markiert
        return "Keine Umbrüche"
            
    except (PermissionError, OSError):
        return f"{YELLOW}Zugriff verweigert{RESET}"

def main():
    parser = argparse.ArgumentParser(description="Prüft Dateien rekursiv auf Zeilenumbrüche.")
    parser.add_argument("path", help="Das zu prüfende Verzeichnis")
    args = parser.parse_args()

    target_dir = Path(args.path)

    if not target_dir.is_dir():
        print(f"Fehler: '{args.path}' ist kein gültiges Verzeichnis.")
        return

    # Header-Länge für die Tabelle anpassen
    print(f"{'DATEIPFAD':<70} | {'ERGEBNIS'}")
    print("-" * 85)

    for file in target_dir.rglob('*'):
        if file.is_file() and file.suffix.lower() not in EXCLUDED_EXTENSIONS:
            result = check_line_endings(file)
            # Ausgabe: Pfad linksbündig, Ergebnis rechts daneben
            print(f"{str(file):<70} | {result}")

if __name__ == "__main__":
    main()
