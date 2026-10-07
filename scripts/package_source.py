"""Regenerate the embedded archive without including build outputs."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

root = Path(__file__).resolve().parents[1] / "src"
files = [root / name for name in ("player.c", "README.txt", "VERIFICATION.txt", "LICENSE.txt", "app.ico", "app.manifest", "app.rc", "build.cmd")]
files += sorted((root / "include").rglob("*.h"))
for path in files:
    if not path.is_file():
        raise FileNotFoundError(path)
with ZipFile(root / "source.zip", "w", ZIP_DEFLATED) as archive:
    for path in files:
        archive.write(path, path.relative_to(root).as_posix())
print("Created src/source.zip")
