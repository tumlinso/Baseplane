#!/usr/bin/env python3
"""Build the scoped reuse consumer against the actual standalone exact core."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--sanitize", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
header = root / "include/Baseplane/incremental/reuse.hh"
fixture = root / "tests/substrate/incremental/test_reuse.cpp"
core = root / "src/seq/dna2_validity.cpp"
with tempfile.TemporaryDirectory(prefix="bp-is1-reuse-") as directory:
    binary = Path(directory) / "reuse-consumer"
    command = ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-I", str(root / "include"),
               str(fixture), str(core), "-o", str(binary)]
    if args.sanitize:
        command[1:1] = ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True)
print(json.dumps({"status": "passed", "sanitized": args.sanitize,
                  "input_sha256": {str(path): hashlib.sha256(path.read_bytes()).hexdigest()
                                   for path in (header, fixture, core)}}, sort_keys=True))
