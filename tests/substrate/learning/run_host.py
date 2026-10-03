#!/usr/bin/env python3
"""Run actual source-conditioned CE fitting and the Baseplane route consumer."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--provider-include", type=Path, required=True)
parser.add_argument("--sanitize", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
provider = args.provider_include.resolve()
provider_header = provider / "learning.hpp"
if not provider_header.is_file():
    parser.error("provider include must contain the actual CE learning.hpp")
header = root / "include/Baseplane/learning/sequence_routes.hh"
fixture = root / "tests/substrate/learning/test_learning.cpp"
core = root / "src/seq/dna2_validity.cpp"
with tempfile.TemporaryDirectory(prefix="bp-is1-learning-") as directory:
    binary = Path(directory) / "learning-consumer"
    command = ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-I", str(root / "include"),
               "-isystem", str(provider), str(fixture), str(core), "-o", str(binary)]
    if args.sanitize:
        command[1:1] = ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True)
print(json.dumps({"status": "passed", "sanitized": args.sanitize,
                  "input_sha256": {str(path): hashlib.sha256(path.read_bytes()).hexdigest()
                                   for path in (header, fixture, core, provider_header)}}, sort_keys=True))
