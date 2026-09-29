#!/usr/bin/env python3
"""Build/run the isolated lab. GPU runs require an already assigned visible device."""
from __future__ import annotations
import argparse,json,os,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=('all','bitlift','scan','rethread','rendezvous')

def invoke(argv,*,capture=False):
    return subprocess.run([str(x) for x in argv],check=True,text=True,capture_output=capture)

def cuda_129_compiler():
    """Select the recorded toolchain even when an older nvcc precedes it in PATH."""
    explicit=os.environ.get('CUDACXX')
    candidates=[Path(explicit)] if explicit else [Path(p)/'nvcc' for p in os.environ.get('PATH','').split(os.pathsep) if p]
    for candidate in candidates:
        if not candidate.is_file():continue
        result=subprocess.run([str(candidate),'--version'],text=True,capture_output=True)
        if result.returncode==0 and 'release 12.9' in result.stdout:return str(candidate.resolve())
    return None

def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--phase',choices=('host','build-cuda','gpu','bench'),required=True)
    p.add_argument('--case',choices=CASES,default='all')
    p.add_argument('--build-dir',type=Path,required=True)
    p.add_argument('--output',type=Path)
    p.add_argument('--n',type=int,default=1025)
    p.add_argument('--iterations',type=int,default=31)
    p.add_argument('--host-sanitize',action='store_true')
    p.add_argument('--sanitizer',choices=('memcheck','initcheck','racecheck','synccheck'))
    a=p.parse_args(argv)
    try:
        if not 0<=a.n<=1048576 or not 1<=a.iterations<=10000:raise ValueError('bounded lab sizes exceeded')
        if a.output and (a.output.suffix!='.json' or a.output.resolve()==ROOT):raise ValueError('output must name a JSON receipt')
        if a.output and a.output.resolve().is_relative_to(ROOT) and not a.output.resolve().is_relative_to(ROOT/'results'):
            raise ValueError('receipts inside the package belong only under results/')
        build=a.build_dir.resolve()
        if build==ROOT or build.is_relative_to(ROOT):raise ValueError('put build output outside the source package')
        if build.exists() and not (build/'CMakeCache.txt').exists() and any(build.iterdir()):raise ValueError('refusing nonempty unrelated build directory')
        cache=build/'CMakeCache.txt'
        if cache.exists() and 'CMAKE_PROJECT_NAME:STATIC=BaseplaneCudaLab' not in cache.read_text():raise ValueError('build directory belongs to another project')
        if a.phase in ('host','build-cuda'):
            cuda=a.phase=='build-cuda'
            compiler=cuda_129_compiler() if cuda else None
            if cuda and not compiler:
                print(json.dumps({'status':'unavailable','reason':'CUDA 12.9 nvcc not found','passed':False}));return 77
            invoke(['cmake','-S',ROOT,'-B',build,f'-DLAB_CUDA={"ON" if cuda else "OFF"}',f'-DLAB_SANITIZE_HOST={"ON" if a.host_sanitize else "OFF"}','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CUDA_ARCHITECTURES=70',*([f'-DCMAKE_CUDA_COMPILER={compiler}'] if cuda else [])])
            invoke(['cmake','--build',build,'--parallel','2','--target','lab_cuda' if cuda else 'lab_host'])
            if cuda:return 0
            cmd=[build/'lab_host',a.case]
        else:
            # Not a lease allocator. These are supplied by Project Control's CUDA
            # scheduler / the existing owner. An environment variable is not proof
            # of exclusivity; the operator must hold the real lease as documented.
            if not os.environ.get('CUDA_VISIBLE_DEVICES'):
                raise ValueError('GPU run needs an externally assigned CUDA_VISIBLE_DEVICES; use the canonical scheduler')
            if not (build/'lab_cuda').is_file():raise ValueError('build-cuda must complete first')
            cmd=[build/'lab_cuda',a.case,str(a.n),str(a.iterations if a.phase=='bench' else 0)]
            if a.sanitizer:
                if a.phase=='bench':raise ValueError('sanitizer timings are not benchmarks')
                compiler=cuda_129_compiler()
                exe=Path(compiler).parents[1]/'compute-sanitizer'/'compute-sanitizer' if compiler else None
                if not exe or not exe.is_file():raise ValueError('matching CUDA 12.9 Compute Sanitizer is required')
                cmd=[exe,'--tool',a.sanitizer,'--error-exitcode','1',*cmd]
        result=invoke(cmd,capture=bool(a.output))
        if a.output:
            out=a.output.resolve()
            if out==ROOT or out.suffix!='.json':raise ValueError('output must name a JSON receipt')
            out.parent.mkdir(parents=True,exist_ok=True)
            out.write_text(json.dumps({'status':'passed','phase':a.phase,'case':a.case,'command':[str(v) for v in cmd],'stdout':result.stdout,'stderr':result.stderr},indent=2)+'\n')
        return 0
    except subprocess.CalledProcessError as e:
        if e.stdout:print(e.stdout,end='')
        if e.stderr:print(e.stderr,file=sys.stderr,end='')
        return e.returncode or 1
    except (OSError,ValueError) as e:
        print(json.dumps({'status':'failed','reason':str(e),'passed':False}),file=sys.stderr);return 1
if __name__=='__main__':raise SystemExit(main())
