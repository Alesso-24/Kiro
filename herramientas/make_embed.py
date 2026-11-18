"""Genera estudio/kiro_embed.html a partir de estudio/kiro_v4.html: solo la criatura (sin panel ni textos),
fondo transparente, controlable desde la pagina que la incrusta con postMessage:
  {t:'expr', id:'feliz'}  -> fija una expresion (desactiva el modo automatico)
  {t:'auto'}              -> vuelve al modo automatico
  {t:'mouse', x, y}       -> hacia donde mira (-1..1); {t:'leave'} suelta la mirada
Uso:  python herramientas/make_embed.py  (desde la raiz del repo)"""
import pathlib
root = pathlib.Path(__file__).resolve().parent.parent
src = (root / 'estudio' / 'kiro_v4.html').read_text(encoding='utf-8')
extra = """
<style>
html,body{background:transparent!important;margin:0;overflow:hidden;height:100%}
header,#panel,#name,#desc,#tour,#pers,.keys{display:none!important}
main{display:block!important;padding:0!important;max-width:none!important;height:100%}
#stage{position:static!important;height:100%;display:grid;place-items:center}
#stage canvas{width:min(calc(100vw - 30px),calc(100vh - 30px))!important;height:auto;aspect-ratio:1;box-shadow:0 0 0 10px #17171b,0 0 0 11px #26262d!important}
</style>
<script>
addEventListener('message',e=>{const m=e.data;if(!m||typeof m!=='object')return;
 if(m.t==='expr'&&EXPS[m.id]){O.auto=false;setExpr(m.id)}
 else if(m.t==='auto'){O.auto=true;setExpr('despierto')}
 else if(m.t==='mouse'){mouse={x:Math.max(-1,Math.min(1,+m.x||0)),y:Math.max(-1,Math.min(1,+m.y||0))}}
 else if(m.t==='leave'){mouse=null}
 else if(m.t==='new'){fresh()}});
</script>
"""
out = src.replace('</html>', extra + '</html>')
assert out != src
(root / 'estudio' / 'kiro_embed.html').write_text(out, encoding='utf-8')
print('ok')
