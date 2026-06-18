from pathlib import Path
import json
import re

try:
    Import("env")
    root = Path(env["PROJECT_DIR"])
except NameError:
    root = Path(__file__).resolve().parents[1]

version = (root / "VERSION").read_text(encoding="utf-8").strip()
platformio = (root / "platformio.ini").read_text(encoding="utf-8")
manifest = json.loads((root / "firmware" / "version.json").read_text(encoding="utf-8"))
public_manifest = json.loads((root / "oba-relays-v1" / "version.json").read_text(encoding="utf-8"))

match = re.search(r'FIRMWARE_VERSION=\\?"([^"\\]+)\\?"', platformio)
if not match:
    raise SystemExit("FIRMWARE_VERSION was not found in platformio.ini")

platformio_version = match.group(1)
manifest_version = str(manifest.get("version", "")).strip()
public_manifest_version = str(public_manifest.get("version", "")).strip()

if version != platformio_version or version != manifest_version or version != public_manifest_version:
    raise SystemExit(
        "Version mismatch: "
        f"VERSION={version}, "
        f"platformio.ini={platformio_version}, "
        f"firmware/version.json={manifest_version}, "
        f"oba-relays-v1/version.json={public_manifest_version}"
    )

print(f"Version check OK: {version}")
