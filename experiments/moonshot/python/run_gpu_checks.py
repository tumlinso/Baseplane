#!/usr/bin/env python3
"""Plan or run tiny GPU comparisons inside a controller-owned foreground lease."""
from __future__ import annotations
import argparse
import hashlib
import json
import math
import subprocess
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bp-build-dir', type=Path, required=True)
    parser.add_argument('--ce-build-dir', type=Path, required=True)
    parser.add_argument('--device', type=int, default=0,
                        help='visible device index; must be zero for the CE tensor harness')
    parser.add_argument('--run', action='store_true', help='execute after the controller assigns resources')
    parser.add_argument('--output', type=Path, help='new JSON receipt; existing files are preserved')
    parser.add_argument('--timeout', type=float, default=120., help='seconds per tiny comparison')
    args = parser.parse_args()
    if args.device != 0:
        parser.error('CE tensor uses visible device zero; the controller must set assigned device visibility')
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error('--timeout must be positive')
    if args.output and args.output.exists():
        parser.error('--output already exists')
    probes = [
        ('baseplane', ['E01','E02','E03','E04'], args.bp_build_dir / 'families/logic/bp_moon_logic_cuda', ['--run','0']),
        ('baseplane', ['E18'], args.bp_build_dir / 'families/rendezvous/bp_moon_rendezvous_cuda_smoke', ['--run','0']),
        ('baseplane', ['E45','E46','E47','E48'], args.bp_build_dir / 'families/machine/bp_moon_machine_cuda_smoke', ['--run','0']),
        ('cellerator', ['E21','E22','E23','E24'], args.ce_build_dir / 'families/tensor/ce_moon_tensor_cuda', []),
    ]
    if args.run:
        missing = [str(binary) for _,_,binary,_ in probes if not binary.is_file()]
        if missing:
            parser.error('missing probe binaries: ' + ', '.join(missing))
    records = []
    for repository, cards, binary, flags in probes:
        command = [str(binary.resolve()), *flags]
        record = {'repository':repository, 'cards':cards, 'argv':command}
        if binary.is_file():
            record['binary_sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
        if args.run:
            try:
                completed = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
                record.update(returncode=completed.returncode, stdout=completed.stdout, stderr=completed.stderr)
            except subprocess.TimeoutExpired as error:
                record.update(returncode=None, timed_out=True,
                              stdout=(error.stdout or b'').decode() if isinstance(error.stdout,bytes) else error.stdout or '',
                              stderr=(error.stderr or b'').decode() if isinstance(error.stderr,bytes) else error.stderr or '')
            except OSError as error:
                record.update(returncode=None, stderr=str(error), stdout='')
            records.append(record)
            if record.get('returncode') != 0:
                break
        else:
            records.append(record)
    passed = args.run and len(records)==len(probes) and all(r.get('returncode')==0 for r in records)
    receipt = {'status':'gpu_comparisons_pass' if passed else 'gpu_comparisons_failed' if args.run else 'planned',
               'executed':args.run, 'benchmark_run':False, 'controller_lease_required':True, 'records':records}
    encoded = json.dumps(receipt, indent=2) + '\n'
    if args.output:
        with args.output.open('x', encoding='utf-8') as output:
            output.write(encoded)
    print(encoded, end='')
    return 0 if passed or not args.run else 1


if __name__ == '__main__':
    raise SystemExit(main())
