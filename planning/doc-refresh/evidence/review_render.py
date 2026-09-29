from io import StringIO
from pathlib import Path
from rich.console import Console
from rich.markdown import Markdown
from PIL import Image
import json

ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'planning/doc-refresh/evidence'
PAGES=[
 'README.md','docs/index.md','docs/design/overview.md','docs/status/current.md',
 'docs/results/index.md','docs/results/bp-carryfold.md','docs/development/start.md',
 'docs/development/source-map.md','CONTRIBUTING.md','AGENTS.md'
]
summary=[]
for width,label in [(96,'normal'),(44,'narrow')]:
    buffer=StringIO()
    console=Console(file=buffer,width=width,color_system=None,force_terminal=False,soft_wrap=False)
    for relative in PAGES:
        console.rule(relative)
        console.print(Markdown((ROOT/relative).read_text(encoding='utf-8')))
    rendered=buffer.getvalue()
    path=OUT/f'reader-render-{label}.txt'
    path.write_text(rendered,encoding='utf-8')
    summary.append({'viewport_columns':width,'label':label,'pages':PAGES,'output':str(path.relative_to(ROOT)),'line_count':rendered.count('\n')})
image=Image.open(ROOT/'docs/results/assets/bp-carryfold.png').convert('L')
gray_path=OUT/'carryfold-grayscale-preview.png'
image.save(gray_path)
summary.append({'grayscale_preview':str(gray_path.relative_to(ROOT)),'dimensions':list(image.size),'mode':image.mode,'derived_from':'docs/results/assets/bp-carryfold.png'})
(OUT/'reader-review.json').write_text(json.dumps({'renderer':'Rich Markdown terminal renderer','browser_available':False,'summary':summary},indent=2)+'\n')
print(json.dumps(summary,indent=2))
