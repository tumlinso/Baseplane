#!/usr/bin/env python3
"""Build/run the real development SDK consumer; retain source hashes and logs."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ce-sdk', required=True, type=Path)
    parser.add_argument('--bp-sdk', required=True, type=Path)
    parser.add_argument('--ce-core-commit', required=True)
    parser.add_argument('--build-dir', type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[3]
    build = args.build_dir.resolve() if args.build_dir else Path(tempfile.mkdtemp(prefix='bp-is1-compose-'))
    build.mkdir(parents=True, exist_ok=True)
    ce, bp = args.ce_sdk.resolve(), args.bp_sdk.resolve()
    commands = [
        ['cmake', '-S', str(Path(__file__).resolve().parent), '-B', str(build),
         '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_PREFIX_PATH=' + str(bp) + ';' + str(ce),
         '-DBaseplane_DIR=' + str(bp / 'lib/cmake/Baseplane'),
         '-DCellerator_DIR=' + str(ce / 'lib/cmake/Cellerator'),
         '-DCE_CORE_COMMIT=' + args.ce_core_commit],
        ['cmake', '--build', str(build), '--parallel', '2'],
        ['ctest', '--test-dir', str(build), '--output-on-failure'],
    ]
    log = build / 'check.log'
    with log.open('w') as output:
        for command in commands:
            output.write(json.dumps(command) + '\n'); output.flush()
            run = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            output.write(run.stdout); output.flush()
            print(run.stdout, end='')
            if run.returncode:
                raise SystemExit(run.returncode)
    observed = (build / 'core-source.txt').read_text().strip()
    if observed != args.ce_core_commit:
        raise SystemExit('installed core source identity mismatch')
    files = [root / 'examples/substrate/sequence_tool/prepared_sequence.hh',
             Path(__file__).resolve().parent / 'consumer.cpp',
             Path(__file__).resolve().parent / 'CMakeLists.txt', Path(__file__).resolve(),
             ce / 'lib/cmake/Cellerator/CelleratorConfig.cmake',
             bp / 'lib/cmake/Baseplane/BaseplaneConfig.cmake']
    files += [bp / 'include/Baseplane' / path for path in [
        'seq/predicate_plan.hh', 'query/contracts/sequence_question.hh', 'representation/hierarchy.hh',
        'index/sequence_index.hh', 'incremental/reuse.hh', 'learning/sequence_routes.hh']]
    files += [ce / 'include/ce_moon/reference.hpp', ce / 'include/ce_moon/mechanisms.hpp',
              ce / 'include/learning.hpp']
    evidence = {'status': 'passed', 'ce_core_commit': observed, 'ce_sdk': str(ce), 'bp_sdk': str(bp),
                'commands': commands, 'log': str(log), 'scope': 'host development sequence composition',
                'input_sha256': {str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in files}}
    (build / 'evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
    print(json.dumps({'status': 'passed', 'evidence': str(build / 'evidence.json'), 'log': str(log)}))


if __name__ == '__main__':
    main()
