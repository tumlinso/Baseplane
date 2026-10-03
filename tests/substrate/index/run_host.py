#!/usr/bin/env python3
"""Compile and run the actual host index consumer with an explicit CE provider."""
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
provider_header = provider / "ce_moon/mechanisms.hpp"
if not provider_header.is_file():
    parser.error("provider include must contain the actual ce_moon/mechanisms.hpp")
test = root / "tests/substrate/index/test_index.cpp"
core = root / "src/seq/dna2_validity.cpp"
header = root / "include/Baseplane/index/sequence_index.hh"
with tempfile.TemporaryDirectory(prefix="bp-is1-index-") as directory:
    binary = Path(directory) / "index-consumer"
    command = ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
               "-I", str(root / "include"), "-isystem", str(provider), str(test), str(core),
               "-o", str(binary)]
    if args.sanitize:
        command[1:1] = ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True)
print(json.dumps({"status": "passed", "sanitized": args.sanitize,
                  "input_sha256": {str(path): hashlib.sha256(path.read_bytes()).hexdigest()
                                   for path in (header, test, core, provider_header)}}, sort_keys=True))
