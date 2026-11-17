#!/usr/bin/env python
"""Genera catalogo/encuesta_rN.html: combinaciones (paleta x especie x pupila) para votar, con animacion.
  python tools/encuesta.py <ronda> [N=60] [semilla]
Usa studio/votos/ronda*.json (exportados de la pagina) para ponderar el muestreo: lo bien votado sale mas, lo
nunca visto sale con peso medio, lo rechazado casi no sale. Nunca repite combinaciones ya votadas."""
import collections, ctypes, glob, importlib.util, json, os, random, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
RONDA = int(sys.argv[1]) if len(sys.argv) > 1 else 2
N = int(sys.argv[2]) if len(sys.argv) > 2 else 60
SEED = int(sys.argv[3]) if len(sys.argv) > 3 else 100 + RONDA
spec = importlib.util.spec_from_file_location("viewer", os.path.join(ROOT, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec)
spec.loader.exec_module(V)
d = V.load()
u32 = ctypes.c_uint32
d.sim_seed.argtypes = [u32]
d.sim_auto.argtypes = [ctypes.c_int]
d.sim_set_species.argtypes = [ctypes.c_int]
d.sim_look.argtypes = [ctypes.c_int, ctypes.c_int]
for f in ("sim_pal_name", "sim_species_name", "sim_pupil_name"):
    getattr(d, f).restype = ctypes.c_char_p
npal, nsp, npu = d.sim_npal(), d.sim_nspecies(), d.sim_npupils()
pn = [d.sim_pal_name(i).decode() for i in range(npal)]
sn = [d.sim_species_name(i).decode() for i in range(nsp)]
un = [d.sim_pupil_name(i).decode() for i in range(npu)]
rng = random.Random(SEED)

# ---- votos anteriores -> pesos ----
SC = {"me_encanta": 2, "mas_o_menos": 1, "no": 0}
st = {k: collections.defaultdict(list) for k in ("pal", "especie", "pupila")}
seen = set()
for fn in sorted(glob.glob(os.path.join(ROOT, "studio", "votos", "ronda*.json"))):
    for x in json.load(open(fn, encoding="utf-8")):
        for k in st:
            st[k][x[k]].append(SC[x["voto"]])
        seen.add((x["pal"], x["especie"], x["pupila"]))
PRIOR = 1.2


def weight(kind, name):                         # media suavizada hacia el prior; 0 votos = prior
    l = st[kind].get(name, [])
    m = (sum(l) + PRIOR * 2) / (len(l) + 2)
    return max(0.05, m) ** 2


wp = [weight("pal", n) for n in pn]
ws = [weight("especie", n) for n in sn]
wu = [weight("pupila", n) for n in un]
IMG = os.path.join(ROOT, "catalogo", f"encuesta_r{RONDA}")
os.makedirs(IMG, exist_ok=True)
for f in glob.glob(os.path.join(IMG, "*")):
    os.remove(f)


def run(t, n):
    for _ in range(n):
        t += 17
        d.sim_frame(t, 1 / 60)
    return t


cards = []
while len(cards) < N:
    c = (rng.choices(range(npal), wp)[0], rng.choices(range(nsp), ws)[0], rng.choices(range(npu), wu)[0])
    key = (pn[c[0]], sn[c[1]], un[c[2]])
    if key in seen:
        continue
    seen.add(key)
    s0 = 5000 * RONDA + len(cards)
    d.sim_init(s0)
    d.sim_auto(0)
    d.sim_seed(s0)
    d.sim_look(c[0], c[2])
    d.sim_set_species(c[1])
    d.sim_set_expr(1, 0, 0)
    t = run(0, 90)
    d.sim_auto(1)
    frs = []
    for _ in range(60):                         # 60 cuadros a 30 fps = 2 s, tiempo real (2 pasos de 17 ms = 33 ms)
        t = run(t, 2)
        frs.append(V.round_img(d, 1).resize((180, 180)))
    fn = f"{len(cards):03d}.webp"
    frs[0].save(os.path.join(IMG, fn), save_all=True, append_images=frs[1:], duration=33, loop=0, lossless=True, method=4)
    frs[0].resize((130, 130)).save(os.path.join(IMG, fn[:-5] + ".png"), optimize=True)
    cards.append({"id": "|".join(key), "pal": key[0], "sp": key[1], "pu": key[2], "img": f"encuesta_r{RONDA}/{fn}", "th": f"encuesta_r{RONDA}/{fn[:-5]}.png"})

html = """<!DOCTYPE html><html lang="es"><meta charset="utf-8"><title>Alloy - ronda __R__</title>
<style>
body{background:#111;color:#ddd;font-family:system-ui,sans-serif;margin:0;display:flex;height:100vh}
#l{flex:1;overflow:auto;padding:12px}#r{width:360px;padding:16px;border-left:1px solid #333;text-align:center;box-sizing:border-box}
h1{font-size:18px;margin:0 0 6px}
button{background:#2a2a35;color:#eee;border:1px solid #444;border-radius:8px;padding:8px 12px;font-size:14px;cursor:pointer;margin:2px}
button:hover{background:#3a3a48}
.g{display:grid;grid-template-columns:repeat(auto-fill,minmax(130px,1fr));gap:10px;margin-top:10px}
.c{border:3px solid #2a2a35;border-radius:50%;cursor:pointer;line-height:0;overflow:hidden;background:#000}
.c img{width:100%;display:block}
.c.s2{border-color:#3fb950}.c.s1{border-color:#d29922}.c.s0{border-color:#f85149;opacity:.45}
.c.sel{outline:3px solid #58a6ff}
#big{width:300px;height:300px;border-radius:50%;background:#000;margin:10px 0}
#nm{font-size:14px;min-height:40px}
.v button{font-size:16px;padding:10px 14px}
small{color:#888}
</style>
<div id="l"><h1>Alloy ronda __R__</h1>
<span id="cnt"></span>
<button onclick="exp()">Descargar JSON</button><button onclick="cp()">Copiar JSON</button>
<button onclick="flt=(flt+1)%4;render()" id="fl"></button>
<button onclick="if(confirm('Borrar todos los votos?')){localStorage.removeItem(K);load();render()}">Borrar</button>
<div class="g" id="g"></div></div>
<div id="r"><img id="big"><div id="nm">Haz clic en una carita</div>
<div class="v"><button onclick="vote(2)">1 Me encanta</button><button onclick="vote(1)">2 Mas o menos</button><button onclick="vote(0)">3 No</button></div>
<textarea id="note" rows="3" placeholder="Por que? (opcional: que te gusta o que cambiarias)" style="width:100%;margin-top:8px;background:#1b1b22;color:#eee;border:1px solid #444;border-radius:8px;padding:6px;box-sizing:border-box" oninput="saveNote()"></textarea>
<small>Sin marcar = ME ENCANTA (solo marca lo que no). Teclas: 2 mas o menos · 3 no · 1 me encanta · Enter siguiente · flechas ← →. Solo se anima la seleccionada.</small></div>
<script>
const CARDS=__CARDS__;const K='alloy_votos_r__R__';let V={},NT={},flt=0,sel=-1,els=[];
const FN=['todas','sin marcar','mas o menos','las que no'];
function load(){try{V=JSON.parse(localStorage.getItem(K)||'{}')}catch(e){V={}}try{NT=JSON.parse(localStorage.getItem(K+'n')||'{}')}catch(e){NT={}}}
function saveNote(){if(sel<0)return;const t=document.getElementById('note').value;if(t)NT[CARDS[sel].id]=t;else delete NT[CARDS[sel].id];localStorage.setItem(K+'n',JSON.stringify(NT))}
function save(){localStorage.setItem(K,JSON.stringify(V))}
function cls(i){const v=V[CARDS[i].id];return 'c'+(v!==undefined?' s'+v:'')+(i==sel?' sel':'')}
function pick(i){sel=i;const c=CARDS[i];document.getElementById('big').src=c.img;
 document.getElementById('nm').textContent=c.pal+' · '+c.sp+' · '+c.pu;document.getElementById('note').value=NT[c.id]||'';els.forEach((e,k)=>{if(e)e.className=cls(k)})}
function vote(v){if(sel<0)return;const id=CARDS[sel].id;if(V[id]===v)delete V[id];else V[id]=v;save();
 const cur=sel;els[cur]&&(els[cur].className=cls(cur));count();if(v!==undefined)next(1)}
function next(d){let k=sel+d;while(k>=0&&k<CARDS.length&&!els[k])k+=d;if(k>=0&&k<CARDS.length){pick(k);els[k].scrollIntoView({block:'nearest'})}}
function count(){document.getElementById('cnt').textContent=Object.keys(V).length+' marcadas (el resto cuenta como me encanta) de '+CARDS.length+' '}
function render(){const g=document.getElementById('g');g.innerHTML='';els=[];
 CARDS.forEach((c,i)=>{const v=V[c.id];
  if((flt==1&&v!==undefined)||(flt==2&&v!==1)||(flt==3&&v!==0)){els.push(null);return}
  const e=document.createElement('div');e.className=cls(i);e.innerHTML='<img src="'+c.th+'" loading="lazy">';e.onclick=()=>pick(i);g.appendChild(e);els.push(e)});
 document.getElementById('fl').textContent='Filtro: '+FN[flt];count()}
function data(){return JSON.stringify(CARDS.map(c=>{const v=V[c.id]===undefined?2:V[c.id];const o={pal:c.pal,especie:c.sp,pupila:c.pu,voto:['no','mas_o_menos','me_encanta'][v]};if(V[c.id]===undefined)o.implicito=true;if(NT[c.id])o.nota=NT[c.id];return o}),null,1)}
function exp(){const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([data()],{type:'application/json'}));a.download='alloy_respuestas_r__R__.json';a.click()}
function cp(){navigator.clipboard.writeText(data()).then(()=>alert('Copiado'))}
document.onkeydown=e=>{if(e.target.tagName=='TEXTAREA')return;if(e.key=='Enter')next(1);else if(e.key=='1')vote(2);else if(e.key=='2')vote(1);else if(e.key=='3')vote(0);else if(e.key=='ArrowRight')next(1);else if(e.key=='ArrowLeft')next(-1)};
load();render();const f=els.findIndex(x=>x);if(f>=0)pick(f);
</script></html>""".replace("__CARDS__", json.dumps(cards)).replace("__R__", str(RONDA))
out = os.path.join(ROOT, "catalogo", f"encuesta_r{RONDA}.html")
open(out, "w", encoding="utf-8").write(html)
print("OK", out, len(cards), "tarjetas,", sum(os.path.getsize(f) for f in glob.glob(os.path.join(IMG, '*'))) // 1024, "KB de animaciones")
