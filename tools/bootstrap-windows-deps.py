"""Fetch pinned build tools into .build/deps without installing system software.

Requires Python 3.14+ (stdlib Zstandard) and Windows curl. Packages retain their
upstream licenses. The MSYS2 tools run only during the build; VBox links its own
bundled libxml2. SDK and MSVC still come from Visual Studio / Windows SDK.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import zipfile


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--offline", action="store_true", help="Verify/extract cached packages only")
    parser.add_argument("--verify-only", action="store_true", help="Verify cached packages/tools without re-extracting")
    args = parser.parse_args()
    if sys.version_info < (3, 14):
        parser.error("Python 3.14+ is required to extract .tar.zst packages")
    tools = Path(__file__).resolve().parent
    dest = tools.parent / ".build" / "deps"
    dest.mkdir(parents=True, exist_ok=True)
    packages = json.loads((tools / "windows-deps.json").read_text(encoding="utf-8"))
    for pkg in packages:
        archive = dest / pkg["file"]
        if not archive.is_file():
            if args.offline or args.verify_only:
                raise FileNotFoundError(f"Missing cached package: {archive}")
            partial = archive.with_name(archive.name + ".part")
            # TLS verification remains enabled. On Windows, allow an offline CRL
            # server; the content is independently pinned by SHA256 below.
            subprocess.run([
                "curl.exe", "--fail", "--location", "--proto", "=https",
                "--proto-redir", "=https", "--ssl-revoke-best-effort", "--retry", "2",
                "--output", str(partial), pkg["url"],
            ], check=True)
            if digest(partial) != pkg["sha256"]:
                raise ValueError(f"SHA256 mismatch: {partial}")
            partial.replace(archive)
        if digest(archive) != pkg["sha256"]:
            raise ValueError(f"SHA256 mismatch: {archive}; inspect before replacing the cache")
        print(f"Verified: {archive.name}", flush=True)
        if "extract" not in pkg or args.verify_only:
            continue
        output = dest / pkg["directory"]
        output.mkdir(exist_ok=True)
        if pkg["extract"] == "zip":
            with zipfile.ZipFile(archive) as bundle:
                for member in bundle.infolist():
                    if not (output / member.filename).resolve().is_relative_to(output.resolve()):
                        raise ValueError(f"Unsafe archive path: {member.filename}")
                bundle.extractall(output)
        else:
            with tarfile.open(archive, "r:zst" if pkg["extract"] == "tar.zst" else "r:gz") as bundle:
                bundle.extractall(output, filter="data")
        # MSYS2 packages share these names; retain metadata for each dependency.
        metadata = dest / "metadata" / pkg["file"]
        metadata.mkdir(parents=True, exist_ok=True)
        for name in (".PKGINFO", ".BUILDINFO", ".MTREE"):
            if (output / name).is_file():
                (metadata / name).write_bytes((output / name).read_bytes())

    # Limit PATH so this check cannot accidentally use unrelated installed DLLs.
    env = os.environ.copy()
    env["PATH"] = str(Path(os.environ["SystemRoot"]) / "System32")
    for exe in (dest / "yasm.exe", dest / "nasm/nasm-2.16.03/nasm.exe", dest / "msys2/mingw64/bin/xsltproc.exe"):
        subprocess.run([str(exe), "--version"], env=env, check=True)
    print(f"Build dependencies ready: {dest}")


if __name__ == "__main__":
    main()
