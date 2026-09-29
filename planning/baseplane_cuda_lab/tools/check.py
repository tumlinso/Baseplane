#!/usr/bin/env python3
"""Check package integrity or evidence completeness, never infer unrun results."""
from __future__ import annotations
import argparse, json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
CASES = ('bitlift', 'scan', 'rethread', 'rendezvous')


def read_json(path: Path) -> dict:
    value = json.loads(path.read_text(encoding='utf-8'))
    if not isinstance(value, dict):
        raise ValueError(f'{path.name}: expected an object')
    return value


def evidence_paths(root: Path, items, label: str) -> list[str]:
    if not isinstance(items, list) or not items:
        raise ValueError(f'{label}: nonempty evidence paths required')
    result = []
    for item in items:
        if not isinstance(item, str) or not item.strip():
            raise ValueError(f'{label}: evidence must name a path')
        p = Path(item)
        if p.is_absolute() or '..' in p.parts:
            raise ValueError(f'{label}: evidence must stay inside the lab')
        resolved = (root / p).resolve()
        if not resolved.is_relative_to(root.resolve()) or not resolved.is_file() or resolved.stat().st_size == 0:
            raise ValueError(f'{label}: missing or unsafe evidence {item}')
        result.append(item)
    return result


def nonempty(value, label: str) -> None:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f'{label}: nonempty text required')


def check_result(root: Path, case: str) -> dict:
    if case not in CASES:
        raise ValueError('unknown experiment')
    record = read_json(root / 'results' / f'{case}.json')
    if record.get('experiment') != case or record.get('status') != 'completed':
        raise ValueError(f'{case}: an actual completed record is required, not a template or blocked run')
    if record.get('verdict') not in {'promote_candidate', 'evaluated_not_promoted'}:
        raise ValueError(f'{case}: verdict must distinguish a candidate from a useful negative result')
    for key in ('correctness', 'sanitizers'):
        value = record.get(key, {})
        if value.get('passed') is not True:
            raise ValueError(f'{case}: {key} did not pass')
        evidence_paths(root, value.get('evidence'), f'{case}/{key}')
    gpu = record.get('cuda', {})
    if gpu.get('compiled') is not True or gpu.get('executed') is not True:
        raise ValueError(f'{case}: CUDA source presence is not compilation/execution evidence')
    evidence_paths(root, gpu.get('evidence'), f'{case}/cuda')
    baselines = record.get('baselines')
    if not isinstance(baselines, list) or not baselines:
        raise ValueError(f'{case}: baseline comparison required')
    for base in baselines:
        if not isinstance(base, dict):
            raise ValueError(f'{case}: each baseline requires name and evidence')
        nonempty(base.get('name'), f'{case}/baseline/name')
        evidence_paths(root, base.get('evidence'), f'{case}/baseline')
    costs, quality = record.get('costs', {}), record.get('quality', {})
    nonempty(costs.get('scope'), f'{case}/costs/scope')
    evidence_paths(root, costs.get('evidence'), f'{case}/costs')
    nonempty(quality.get('probe'), f'{case}/quality/probe')
    evidence_paths(root, quality.get('evidence'), f'{case}/quality')
    if not isinstance(record.get('limitations'), list) or not record['limitations']:
        raise ValueError(f'{case}: limitations must remain explicit')
    for limit in record['limitations']:
        nonempty(limit, f'{case}/limitation')
    nonempty(record.get('conclusion'), f'{case}/conclusion')
    return {'experiment': case, 'evidence_complete': True, 'verdict': record['verdict']}


def check_selection(root: Path, results: list[dict]) -> dict:
    value = read_json(root / 'results' / 'selection.json')
    if value.get('status') != 'completed':
        raise ValueError('selection: actual review required')
    selected = value.get('selected')
    if not isinstance(selected, list) or len(selected) > 2 or len(selected) != len(set(selected)) or not set(selected) <= set(CASES):
        raise ValueError('selection: choose zero, one or two unique experiment IDs')
    candidates = {r['experiment'] for r in results if r['verdict'] == 'promote_candidate'}
    if not set(selected) <= candidates:
        raise ValueError('selection cannot promote a negative result')
    nonempty(value.get('rationale'), 'selection/rationale')
    evidence_paths(root, value.get('evidence'), 'selection/report')
    if selected:
        probe = value.get('two_level_probe', {})
        if probe.get('executed') is not True:
            raise ValueError('selected mechanisms require a bounded two-level demonstration')
        evidence_paths(root, probe.get('evidence'), 'selection/two_level_probe')
    return {'selected': selected, 'evidence_complete': True}


def check_package(root: Path) -> dict:
    required = ['START_HERE.md', 'RESEARCH.md', 'DOCTRINE.md', 'BENCHMARK_PROTOCOL.md',
                'CMakeLists.txt', 'include/lab.hh', 'host/reference.cc', 'cuda/kernels.cuh',
                'cuda/driver.cu', 'tools/install.py', 'tools/run.py', 'tools/check.py',
                'planning/epic.v2.json', 'planning/scoped_policy_delta.json', 'evidence/sources.json']
    required += [f'experiments/{case}.md' for case in CASES]
    for name in required:
        if not (root / name).is_file():
            raise ValueError(f'missing package source: {name}')
    plan = read_json(root / 'planning' / 'epic.v2.json')
    if plan.get('schema_version') != 2 or plan.get('project', {}).get('name') != 'baseplane':
        raise ValueError('wrong plan version or authority')
    tasks = plan.get('tasks', [])
    ids = [task['id'] for task in tasks]
    if len(ids) != 7 or len(set(ids)) != len(ids):
        raise ValueError('expected one epic and six unique outcome tasks')
    by_id = {task['id']: task for task in tasks}
    if sum(task.get('kind') == 'epic' for task in tasks) != 1:
        raise ValueError('expected exactly one epic')
    for task in tasks:
        if task.get('parallel_policy') != 'serial':
            raise ValueError('default execution is serial')
        if task.get('parent_id') and task['parent_id'] not in by_id:
            raise ValueError('missing parent')
        scope = task.get('scope', {})
        if task.get('kind') != 'epic' and scope.get('exclusive_paths') != ['experiments/cuda_lab']:
            raise ValueError('task write scope escaped the experiment lab')
        for gate in task.get('gates', []):
            if gate.get('type') != 'command' or not gate.get('argv'):
                raise ValueError('invalid command gate')
            for part in gate['argv']:
                if isinstance(part, str) and part.startswith('experiments/cuda_lab/') and part.endswith('.py'):
                    if not (root / part.removeprefix('experiments/cuda_lab/')).is_file():
                        raise ValueError(f'gate names missing script {part}')
            if 'cuda' in gate and gate['cuda'].get('gpus') != 1:
                raise ValueError('minimal prototype gates use one assigned GPU')
    done, active = set(), set()
    def visit(tid):
        if tid in active:
            raise ValueError('task dependency cycle')
        if tid in done:
            return
        active.add(tid)
        for dependency in by_id[tid].get('depends_on', []):
            target = dependency.get('task_id')
            if dependency.get('type') != 'task' or target not in by_id:
                raise ValueError('unknown dependency')
            visit(target)
        active.remove(tid); done.add(tid)
    for tid in by_id:
        visit(tid)
    sources = read_json(root / 'evidence' / 'sources.json')
    return {'status': 'passed', 'files_checked': len(required), 'native_task_count': len(tasks),
            'scope': 'local structural checks; not native execution, CUDA validation or experimental results'}


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--package', action='store_true')
    modes.add_argument('--result', choices=CASES)
    modes.add_argument('--all-results', action='store_true')
    args = parser.parse_args(argv)
    try:
        if args.result:
            result = check_result(ROOT, args.result)
        elif args.all_results:
            results = [check_result(ROOT, case) for case in CASES]
            result = {'results': results, 'selection': check_selection(ROOT, results)}
        else:
            result = check_package(ROOT)
        print(json.dumps(result, indent=2)); return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(json.dumps({'status': 'failed', 'reason': str(exc)}), file=sys.stderr); return 1
if __name__ == '__main__':
    raise SystemExit(main())
