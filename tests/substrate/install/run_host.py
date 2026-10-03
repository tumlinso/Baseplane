#!/usr/bin/env python3
"""Qualify the actual installed exact core or require the integrated host SDK."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--baseplane-prefix", type=Path)
parser.add_argument("--ce-prefix", type=Path)
parser.add_argument("--ce-package", default="Cellerator")
parser.add_argument("--require-integrated", action="store_true")
args = parser.parse_args()
if args.require_integrated and (not args.baseplane_prefix or not args.ce_prefix):
    parser.error("--require-integrated requires actual --baseplane-prefix and --ce-prefix")
root = Path(__file__).resolve().parents[3]
owned = [root / "cmake/substrate/BaseplaneSubstrate.cmake",
         root / "examples/substrate/install/CMakeLists.txt",
         root / "examples/substrate/install/consumer.cpp",
         root / "tests/substrate/install/missing-provider/CMakeLists.txt"]
def run(command, expected_error=None):
    completed = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if expected_error is None:
        if completed.returncode:
            print(completed.stdout)
            raise subprocess.CalledProcessError(completed.returncode, command)
    elif completed.returncode == 0 or " ".join(expected_error.split()) not in " ".join(completed.stdout.split()):
        print(completed.stdout)
        raise RuntimeError("required capability rejection missing: " + expected_error)
    return completed.stdout

with tempfile.TemporaryDirectory(prefix="bp-is1-install-") as temporary:
    work = Path(temporary)
    if args.baseplane_prefix:
        prefix = args.baseplane_prefix.resolve()
    else:
        build = work / "core-build"
        installed = work / "original-prefix"
        run(["cmake", "-S", str(root), "-B", str(build), "-DBASEPLANE_ENABLE_CUDA=OFF",
             "-DBASEPLANE_ENABLE_HIGHWAY=OFF", "-DCMAKE_BUILD_TYPE=Debug"])
        run(["cmake", "--build", str(build), "--target", "baseplane_seq", "-j", "2"])
        run(["cmake", "--install", str(build), "--prefix", str(installed)])
        prefix = work / "relocated-prefix"
        installed.rename(prefix)
    package_configs = list(prefix.glob("lib*/cmake/Baseplane/BaseplaneConfig.cmake"))
    if len(package_configs) != 1:
        raise RuntimeError("expected one installed Baseplane package config")
    package = package_configs[0].parent
    cmake_prefix = [str(prefix)]
    if args.ce_prefix:
        cmake_prefix.append(str(args.ce_prefix.resolve()))
    configure = ["cmake", "-S", str(root / "examples/substrate/install"), "-B", str(work / "consumer"),
                 "-DBaseplane_DIR=" + str(package), "-DCMAKE_PREFIX_PATH=" + ";".join(cmake_prefix),
                 "-DBP_CE_PACKAGE=" + args.ce_package, "-DCMAKE_BUILD_TYPE=Debug",
                 "-DBP_REQUIRE_INTEGRATED=" + ("ON" if args.require_integrated else "OFF")]
    run(configure)
    run(["cmake", "--build", str(work / "consumer"), "--target", "baseplaneInstalledConsumer", "-j", "2"])
    print(run(["ctest", "--test-dir", str(work / "consumer"),
               "-R", "^baseplaneInstalledConsumer$", "--output-on-failure"]).strip())
    if not args.require_integrated:
        run(["cmake", "-S", str(root / "tests/substrate/install/missing-provider"),
             "-B", str(work / "missing-provider"), "-DBaseplane_DIR=" + str(package),
             "-DSUBSTRATE_MODULE=" + str(root / "cmake/substrate/BaseplaneSubstrate.cmake")],
            "Missing selected installed CE capability")
        # Required integration must never pass on an unintegrated standalone SDK.
        if not (package / "BaseplaneSubstrate.cmake").is_file():
            run(configure[:configure.index("-B") + 1] + [str(work / "must-integrate")]
                + configure[configure.index("-B") + 2:-1] + ["-DBP_REQUIRE_INTEGRATED=ON"],
                "Installed Baseplane substrate module unavailable")
        run(configure[:configure.index("-B") + 1] + [str(work / "must-cuda")]
            + configure[configure.index("-B") + 2:] + ["-DBP_REQUIRE_CUDA=ON"],
            "qualification consumer is CPU-only")
    else:
        run(configure[:configure.index("-B") + 1] + [str(work / "must-cuda")]
            + configure[configure.index("-B") + 2:] + ["-DBP_REQUIRE_CUDA=ON"],
            "CUDA execution is unavailable")
    installed_inputs = [package_configs[0], package / "BaseplaneTargets.cmake",
                        prefix / "include/Baseplane/seq/dna2_validity.hh"]
    installed_inputs += list(prefix.glob("lib*/libbaseplane_seq.a"))
    if args.require_integrated:
        installed_inputs.append(package / "BaseplaneSubstrate.cmake")
        for component_header in ("query/contracts/sequence_question.hh", "representation/hierarchy.hh",
                                 "index/sequence_index.hh", "incremental/reuse.hh", "learning/sequence_routes.hh"):
            installed_inputs.append(prefix / "include/Baseplane" / component_header)
        installed_inputs += list(args.ce_prefix.resolve().glob("lib*/cmake/" + args.ce_package + "/*.cmake"))
    print(json.dumps({"status": "passed", "integrated": args.require_integrated,
                      "relocated_fresh_core": args.baseplane_prefix is None,
                      "input_sha256": {str(path): hashlib.sha256(path.read_bytes()).hexdigest()
                                       for path in owned + installed_inputs}}, sort_keys=True))
