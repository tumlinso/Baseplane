#!/usr/bin/env python3
"""Add this entire lab under experiments/cuda_lab; default is a non-mutating preview."""
from __future__ import annotations
import argparse,hashlib,json,os,shutil,sys,tempfile
from pathlib import Path
SOURCE=Path(__file__).resolve().parents[1]
EXCLUDE={'__pycache__','.git'}
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def install(repo:Path,apply:bool=False):
    repo=repo.resolve(strict=True)
    if not (repo/'CMakeLists.txt').is_file() or not (repo/'AGENTS.md').is_file():raise ValueError('expected the actual Baseplane repository')
    if 'Baseplane' not in (repo/'CMakeLists.txt').read_text():raise ValueError('repository does not look like Baseplane')
    parent=repo/'experiments';target=parent/'cuda_lab'
    if parent.is_symlink() or target.is_symlink():raise ValueError('destination symlinks are not allowed')
    files=[p for p in SOURCE.rglob('*') if p.is_file() and not any(part in EXCLUDE for part in p.relative_to(SOURCE).parts)]
    if any(p.is_symlink() for p in SOURCE.rglob('*')):raise ValueError('source symlink is not supported')
    if target.exists():
        expected={p.relative_to(SOURCE).as_posix():digest(p) for p in files}
        found={p.relative_to(target).as_posix():digest(p) for p in target.rglob('*') if p.is_file()}
        if expected==found:return {'status':'already_identical','target':str(target),'applied':False}
        raise ValueError('destination exists with different content; reconcile explicitly, never overwrite')
    result={'status':'preview','target':str(target),'files':len(files),'applied':False}
    if apply:
        parent.mkdir(exist_ok=True)
        stage=Path(tempfile.mkdtemp(prefix='.cuda_lab-stage-',dir=parent))
        try:
            for src in files:
                dst=stage/src.relative_to(SOURCE);dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
            if target.exists():raise ValueError('destination appeared during installation')
            os.rename(stage,target)
        except BaseException:
            shutil.rmtree(stage,ignore_errors=True);raise
        result.update(status='installed',applied=True)
    return result
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True);p.add_argument('--apply',action='store_true');a=p.parse_args()
    try:print(json.dumps(install(a.repo,a.apply),indent=2))
    except (OSError,ValueError) as e:print(str(e),file=sys.stderr);raise SystemExit(1)
