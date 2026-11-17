"""Genera el sitio de GitHub Pages (carpeta docs/): capitulos en HTML a partir de docs/*.md y la portada docs/index.html.
Uso (desde la raiz del repo):  python herramientas/build_site.py
Pages se publica desde la rama main, carpeta /docs (con .nojekyll)."""
import re, pathlib, markdown

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / 'docs'
OUT = DOCS / 'capitulos'
OUT.mkdir(exist_ok=True)
GH = 'https://github.com/Alesso-24/Kiro'

CSS = """
:root{--bg:#0b0b0d;--panel:#131317;--line:#24242b;--tx:#e8e3d8;--mut:#8f8a80;--ac:#4de3ff}
*{box-sizing:border-box}html,body{margin:0;background:var(--bg);color:var(--tx);font:16px/1.6 system-ui,Segoe UI,sans-serif}
a{color:var(--ac)}
nav.top{position:sticky;top:0;z-index:5;background:#0b0b0dee;backdrop-filter:blur(6px);border-bottom:1px solid var(--line);padding:10px 22px;display:flex;gap:18px;flex-wrap:wrap;align-items:center}
nav.top b{font-size:18px}nav.top a{text-decoration:none;font-size:14px;color:var(--mut)}nav.top a:hover{color:var(--ac)}
main{max-width:900px;margin:0 auto;padding:26px 22px 80px}
h1,h2,h3{line-height:1.25}h1{font-size:30px}h2{margin-top:2em;border-bottom:1px solid var(--line);padding-bottom:6px}
img{max-width:100%;height:auto;border-radius:10px;border:1px solid var(--line)}
table{border-collapse:collapse;width:100%;font-size:14px;margin:14px 0;display:block;overflow-x:auto}
th,td{border:1px solid var(--line);padding:7px 10px;text-align:left;vertical-align:top}th{background:#17171b}
code{background:#17171b;padding:1px 6px;border-radius:5px;font-size:.92em}
pre{background:#101014;border:1px solid var(--line);border-radius:10px;padding:14px;overflow:auto}pre code{background:none;padding:0}
blockquote{margin:14px 0;padding:8px 16px;border-left:3px solid var(--ac);background:#101014;color:#cfc9bd;border-radius:0 8px 8px 0}
em{color:#b9b3a6}
.pager{display:flex;justify-content:space-between;margin-top:50px;border-top:1px solid var(--line);padding-top:16px}
"""

CHAPTERS = [
    ('01-concepto', 'Concepto'), ('02-hardware', 'Hardware y BOM'), ('03-esquematico', 'Esquemático'),
    ('04-pcb', 'PCB'), ('05-fabricacion', 'Fabricación'), ('06-ensamble', 'Ensamble'),
    ('07-firmware', 'Firmware'), ('08-animaciones', 'Animaciones'), ('09-inconsistencias', 'Inconsistencias'),
    ('10-pendientes', 'Pendientes y bitácora'), ('11-carcasa', 'Carcasa'),
]


def fix_links(html):
    html = html.replace('src="img/', 'src="../img/')
    html = re.sub(r'href="(\d\d-[a-z-]+)\.md(#[^"]*)?"', r'href="\1.html\2"', html)

    def rel(m):
        p = m.group(1)
        if p.startswith('estudio/'):
            return 'href="../' + p + '"'
        kind = 'tree' if p.endswith('/') else 'blob'
        return f'href="{GH}/{kind}/main/{p}"'
    html = re.sub(r'href="\.\./([^"]+)"', rel, html)
    html = html.replace('href="img/', 'href="../img/')
    return html


def page(title, body, prev=None, nxt=None):
    nav = '<nav class="top"><b>Kiro</b><a href="../index.html">Inicio</a><a href="../estudio/kiro_v4.html">Estudio de animaciones</a><a href="../estudio/intro_kiro_10.html">Intros</a>'
    nav += ''.join(f'<a href="{c}.html">{n}</a>' for c, n in CHAPTERS) + f'<a href="{GH}">GitHub</a></nav>'
    pager = '<div class="pager"><span>' + (f'← <a href="{prev[0]}.html">{prev[1]}</a>' if prev else '') + '</span><span>' + (f'<a href="{nxt[0]}.html">{nxt[1]}</a> →' if nxt else '') + '</span></div>'
    return f'<!doctype html><html lang="es"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>{title} · Kiro</title><style>{CSS}</style>{nav}<main>{body}{pager}</main></html>'


md = markdown.Markdown(extensions=['tables', 'fenced_code', 'sane_lists'])
for i, (stem, name) in enumerate(CHAPTERS):
    text = (DOCS / f'{stem}.md').read_text(encoding='utf-8')
    md.reset()
    body = fix_links(md.convert(text))
    prev = CHAPTERS[i - 1] if i else None
    nxt = CHAPTERS[i + 1] if i + 1 < len(CHAPTERS) else None
    (OUT / f'{stem}.html').write_text(page(name, body, prev, nxt), encoding='utf-8')
print('capitulos:', len(CHAPTERS))
