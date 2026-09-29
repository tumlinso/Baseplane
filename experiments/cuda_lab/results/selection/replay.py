import json,subprocess,sys
from pathlib import Path
REPO=Path(__file__).resolve().parents[4]
BUILD='build-bitop-BP-CUDA-LAB-final-cuda'
ROOT='/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9'
CONTROLLER=str(Path.home()/'.agents/skills/cuda/scripts/cuda_controller.py')

def child():
    folder=REPO/'experiments/cuda_lab/results/selection'
    folder.mkdir(parents=True,exist_ok=True)
    for mode in ('memcheck','initcheck','racecheck','synccheck'):
        output=folder/f'final-{mode}.json'
        command=['python3','experiments/cuda_lab/tools/run.py','--phase','gpu','--case','all','--n','1025','--build-dir',BUILD,'--sanitizer',mode,'--output',str(output)]
        print(mode,flush=True)
        if subprocess.run(command,cwd=REPO).returncode:return 1
    for size in (1025,65536):
        output=folder/f'final-bench-{size}.json'
        command=['python3','experiments/cuda_lab/tools/run.py','--phase','bench','--case','all','--n',str(size),'--iterations','31','--build-dir',BUILD,'--output',str(output)]
        print(size,flush=True)
        if subprocess.run(command,cwd=REPO).returncode:return 1
    return 0

def parent():
    spec={'schema_version':1,'project_root':str(REPO),'argv':[sys.executable,__file__,'child'],'recipe':'baseline','campaign_id':'BP-CUDA-LAB-final-replay','resources':{'gpus':1},'toolchain':{'root':ROOT,'require_sanitizer':True},'quiescence':{'timeout_seconds':45,'consecutive_idle_samples':3},'binary_paths':[f'{BUILD}/lab_cuda'],'paths':['experiments/cuda_lab/cuda/driver.cu','experiments/cuda_lab/cuda/kernels.cuh','experiments/cuda_lab/tools/run.py']}
    result=subprocess.run([sys.executable,CONTROLLER,'run','--spec','-','--json'],input=json.dumps(spec),text=True,capture_output=True,cwd=REPO)
    print(result.stdout.strip())
    if result.stderr:print(result.stderr[-1600:],file=sys.stderr)
    return 0 if result.returncode==0 and json.loads(result.stdout).get('ok') is True else 1

if __name__=='__main__':raise SystemExit(parent() if sys.argv[1]=='parent' else child())
