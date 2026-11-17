"""Simulador de Alloy (tamagotchi Lil Guy): corre el MISMO codigo de firmware/Alloy_Eyes/EyesCore.h en la PC.

  python viewer.py                 ventana interactiva (recompila sola la DLL si cambio EyesCore.h)
  python viewer.py --snap DIR      guarda una hoja con todas las expresiones y sale (sin ventana)

EXPRESIONES (teclas)
  1 despierto  2 curioso  3 dormido  4 feliz  5 enojado  6 amor  7 cargando  8 electrico  9 dormido-tenue
  0 triste  C cara llena  H hype  B bateria baja  M mareado  N sorprendido  O orgulloso
  Y bostezo
  ESPACIO  siguiente expresion (recorre todas)

TAMAGOTCHI (como si fueran los sensores / botones)
  clic izq        TOCAR (3 toques seguidos = amor)       mantener clic der   ACARICIAR (amor)
  G  jugar (boton verde)   -> hype, sube diversion       U  enchufar / desenchufar USB (cargar)
  F  bateria llena (cargando)                             L  bateria baja on/off
  K  sacudida suave (curioso)    Shift+K  sacudida fuerte (mareo + enojo)
  P  piquete    W  guino    ENTER  nuevo LOOK (paleta + pupila)
  J  ruido fuerte (ansioso)   I  frio on/off (tirita)   T  pulgar arriba (ver bateria)   X  desprecio (triste)
  mouse: los ojos miran el cursor; mover muy rapido no marea (eso lo hace K)
  + / -  velocidad del tiempo (x1 x5 x20) para ver como se cansa / se aburre / duerme
  Q/A  pupila izq +/-   E/D  pupila der +/-   R  pupilas auto
  S  captura PNG a sim/shots     ESC  salir
"""
import ctypes, os, subprocess, sys, time
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
CORE = os.path.join(HERE, "..", "firmware", "Alloy_Eyes", "EyesCore.h")
SRC = os.path.join(HERE, "eyes_sim.cpp")
BUILD = os.path.join(HERE, "build")
W = H = 240

KEY_EXPR = {"1": 1, "2": 2, "3": 3, "4": 4, "5": 5, "6": 6, "7": 7, "8": 8, "9": 9, "0": 10,
            "c": 11, "h": 12, "b": 13, "m": 14, "n": 15, "o": 16, "y": 17}


def build():
    """compila a un nombre unico por version (asi nunca choca con una ventana abierta)"""
    os.makedirs(BUILD, exist_ok=True)
    fw = os.path.dirname(CORE)
    mt = [os.path.getmtime(SRC)]
    for r, _, fs in os.walk(fw):          # cualquier .h (core, Motion, Data y datos/) invalida la DLL
        mt += [os.path.getmtime(os.path.join(r, f)) for f in fs if f.endswith((".h", ".txt"))]
    stamp = int(max(mt))
    dll = os.path.join(BUILD, f"eyes_{stamp}.dll")
    if not os.path.exists(dll):
        print("compilando", os.path.basename(dll), "...")
        r = subprocess.run(["g++", "-O2", "-shared", "-static-libgcc", "-static-libstdc++", "-o", dll, SRC],
                           capture_output=True, text=True)
        if r.returncode:
            print(r.stderr)
            sys.exit(1)
        for f in os.listdir(BUILD):   # limpia versiones viejas que no esten en uso
            if f.endswith(".dll") and f != os.path.basename(dll):
                try:
                    os.remove(os.path.join(BUILD, f))
                except OSError:
                    pass
    return dll


def load():
    d = ctypes.CDLL(build())
    u32, f32, i32 = ctypes.c_uint32, ctypes.c_float, ctypes.c_int
    d.sim_buf.restype = ctypes.POINTER(ctypes.c_uint16)
    d.sim_name.restype = ctypes.c_char_p
    d.sim_name.argtypes = [i32]
    d.sim_frame.argtypes = [u32, f32]
    d.sim_init.argtypes = [u32]
    d.sim_set_expr.argtypes = [i32, u32, u32]
    d.sim_gaze.argtypes = [f32, f32, u32, u32]
    d.sim_pupil.argtypes = [f32, f32]
    d.sim_look.argtypes = [i32, i32]
    d.sim_timescale.argtypes = [f32]
    d.sim_pet.argtypes = [u32, i32]
    d.sim_shake.argtypes = [u32, i32]
    d.sim_set_usb.argtypes = [u32, i32]
    d.sim_cold.argtypes = [u32, i32]
    d.sim_batlow.argtypes = [u32, i32]
    for n in ("sim_loud", "sim_thumbs", "sim_flick", "sim_next_expr", "sim_next_theme", "sim_poke", "sim_wink", "sim_tap", "sim_play", "sim_batfull"):
        getattr(d, n).argtypes = [u32]
    d.sim_stats.argtypes = [ctypes.POINTER(ctypes.c_float)]
    return d


def frame_rgb(d):
    a = np.ctypeslib.as_array(d.sim_buf(), shape=(H, W)).astype(np.uint32)
    r = ((a >> 11) & 31) * 255 // 31
    g = ((a >> 5) & 63) * 255 // 63
    b = (a & 31) * 255 // 31
    return np.stack([r, g, b], axis=2).astype(np.uint8)  # (H, W, 3)


def run_frames(d, t_ms, n, dt=1 / 60):
    for _ in range(n):
        t_ms += int(dt * 1000 + 0.5)
        d.sim_frame(t_ms, dt)
    return t_ms


def round_img(d, scale=1):
    """cuadro con la mascara redonda de la pantalla"""
    from PIL import Image, ImageDraw
    im = Image.fromarray(frame_rgb(d))
    if scale != 1:
        im = im.resize((W * scale, H * scale), Image.LANCZOS)
    m = Image.new("L", im.size, 0)
    ImageDraw.Draw(m).ellipse((0, 0, im.size[0] - 1, im.size[1] - 1), fill=255)
    bg = Image.new("RGB", im.size, (40, 40, 46))
    bg.paste(im, (0, 0), m)
    return bg


def snap(outdir):
    from PIL import Image, ImageDraw
    os.makedirs(outdir, exist_ok=True)
    d = load()
    d.sim_init(1234)
    t = run_frames(d, 0, 150)
    tiles = []
    for x in range(1, d.sim_nexpr()):
        d.sim_set_expr(x, t, 0)
        t = run_frames(d, t, 100)       # deja que termine el morph y se vea el estado estable
        im = round_img(d, 1)
        tile = Image.new("RGB", (240, 262), (24, 24, 28))
        tile.paste(im, (0, 0))
        ImageDraw.Draw(tile).text((8, 246), f"{x} {d.sim_name(x).decode()}", fill=(230, 230, 230))
        tiles.append(tile)
    cols = 4
    sheet = Image.new("RGB", (cols * 240, 5 * 262), (24, 24, 28))
    for i, tl in enumerate(tiles):
        sheet.paste(tl, ((i % cols) * 240, (i // cols) * 262))
    sheet.save(os.path.join(outdir, "hoja.png"))
    print("guardado", os.path.join(outdir, "hoja.png"))


def film(outdir, seqspec):
    """tira de cuadros: seqspec = lista de (expresion, segundos) -> una fila de miniaturas cada 0.2 s"""
    from PIL import Image
    os.makedirs(outdir, exist_ok=True)
    d = load()
    d.sim_init(7)
    t = run_frames(d, 0, 150)
    frames = []
    for x, secs in seqspec:
        d.sim_set_expr(x, t, 0)
        n = int(secs / 0.2)
        for _ in range(n):
            t = run_frames(d, t, 12)
            frames.append(round_img(d, 1).resize((120, 120), Image.LANCZOS))
    cols = 10
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * 120, rows * 120), (24, 24, 28))
    for i, f in enumerate(frames):
        sheet.paste(f, ((i % cols) * 120, (i // cols) * 120))
    fn = os.path.join(outdir, "tira.png")
    sheet.save(fn)
    print("guardado", fn)


def gallery(outdir):
    """16 looks (paleta x estilo de pupila) en estado despierto"""
    from PIL import Image
    os.makedirs(outdir, exist_ok=True)
    d = load()
    d.sim_init(99)
    d.sim_set_expr(1, 0, 0)
    t = run_frames(d, 0, 120)
    tiles = []
    for i in range(16):
        d.sim_look(i, (i * 3 + 1) % 7)
        t = run_frames(d, t, 140)
        tiles.append(round_img(d, 1).resize((180, 180), Image.LANCZOS))
    sheet = Image.new("RGB", (4 * 180, 4 * 180), (24, 24, 28))
    for i, im in enumerate(tiles):
        sheet.paste(im, ((i % 4) * 180, (i // 4) * 180))
    fn = os.path.join(outdir, "galeria.png")
    sheet.save(fn)
    print("guardado", fn)


def idle_film(outdir, secs):
    """deja al bicho solo (modo autonomo) y guarda un cuadro cada 0.25 s"""
    from PIL import Image
    os.makedirs(outdir, exist_ok=True)
    d = load()
    d.sim_init(31337)
    t = 0
    frames = []
    for _ in range(int(secs / 0.25)):
        t = run_frames(d, t, 15)
        frames.append(round_img(d, 1).resize((100, 100), Image.LANCZOS))
    cols = 16
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * 100, rows * 100), (24, 24, 28))
    for i, im in enumerate(frames):
        sheet.paste(im, ((i % cols) * 100, (i // cols) * 100))
    fn = os.path.join(outdir, "idle.png")
    sheet.save(fn)
    print("guardado", fn)


def export(outdir, secs=3.6, fps=25):
    """guarda cada expresion como GIF animado (con el morph de entrada) + una hoja de contacto"""
    from PIL import Image
    os.makedirs(outdir, exist_ok=True)
    d = load()
    names = []
    for x in range(1, d.sim_nexpr()):
        d.sim_init(2024)
        d.sim_look(0, 5)
        t = run_frames(d, 0, 400)            # termina el arranque, queda despierto
        d.sim_set_expr(x, t, 0)
        frames = []
        for _ in range(int(secs * fps)):
            t = run_frames(d, t, 1, 1 / fps)
            frames.append(round_img(d, 2).convert("P", palette=Image.ADAPTIVE, colors=64))
        name = d.sim_name(x).decode()
        fn = os.path.join(outdir, f"{x:02d}_{name}.gif")
        frames[0].save(fn, save_all=True, append_images=frames[1:], duration=int(1000 / fps), loop=0, disposal=2)
        names.append(name)
        print(fn)
    return names


def main():
    if "--export" in sys.argv:
        export(sys.argv[sys.argv.index("--export") + 1])
        return
    if "--gallery" in sys.argv:
        gallery(os.path.join(HERE, "shots"))
        return
    if "--idle" in sys.argv:
        idle_film(os.path.join(HERE, "shots"), float(sys.argv[sys.argv.index("--idle") + 1]))
        return
    if "--film" in sys.argv:
        spec = [(int(a), float(b)) for a, b in (p.split(":") for p in sys.argv[sys.argv.index("--film") + 1].split(","))]
        film(os.path.join(HERE, "shots"), spec)
        return
    if "--snap" in sys.argv:
        snap(sys.argv[sys.argv.index("--snap") + 1])
        return
    import pygame
    d = load()
    d.sim_init(int(time.time()) & 0xFFFFFF)
    pygame.init()
    S = 3
    size = W * S
    HUD = 120
    screen = pygame.display.set_mode((size, size + HUD))
    pygame.display.set_caption("Alloy - tamagotchi Lil Guy (simulador)")
    font = pygame.font.SysFont("consolas", 16)
    small = pygame.font.SysFont("consolas", 13)
    mask = pygame.Surface((size, size), pygame.SRCALPHA)
    mask.fill((24, 24, 28, 255))
    pygame.draw.circle(mask, (0, 0, 0, 0), (size // 2, size // 2), size // 2)
    clock = pygame.time.Clock()
    t0 = time.time()
    lastt = t0
    pl = pr = 1.0
    manual = False
    scales = [1, 5, 20]
    si = 0
    msg = ""
    msg_t = 0
    stats = (ctypes.c_float * 3)()
    usb = False
    lowbat = False
    cold = False
    petting = False
    while True:
        now = time.time()
        t_ms = int((now - t0) * 1000)
        dt = min(now - lastt, 0.04)
        lastt = now
        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                return
            if ev.type == pygame.KEYDOWN:
                k = ev.key
                name = pygame.key.name(k)
                mods = pygame.key.get_mods()
                if k == pygame.K_ESCAPE:
                    return
                elif k == pygame.K_SPACE:
                    d.sim_next_expr(t_ms)
                elif k == pygame.K_RETURN:
                    d.sim_next_theme(t_ms)
                elif name in KEY_EXPR:
                    d.sim_set_expr(KEY_EXPR[name], t_ms, 0 if KEY_EXPR[name] in (3, 9, 11) else 4000)
                elif k == pygame.K_j:
                    d.sim_loud(t_ms)
                elif k == pygame.K_i:
                    cold = not cold
                    d.sim_cold(t_ms, 1 if cold else 0)
                elif k == pygame.K_t:
                    d.sim_thumbs(t_ms)
                elif k == pygame.K_x:
                    d.sim_flick(t_ms)
                elif k == pygame.K_w:
                    d.sim_wink(t_ms)
                elif k == pygame.K_p:
                    d.sim_poke(t_ms)
                elif k == pygame.K_g:
                    d.sim_play(t_ms)
                elif k == pygame.K_u:
                    usb = not usb
                    d.sim_set_usb(t_ms, 1 if usb else 0)
                elif k == pygame.K_f:
                    d.sim_batfull(t_ms)
                elif k == pygame.K_l:
                    lowbat = not lowbat
                    d.sim_batlow(t_ms, 1 if lowbat else 0)
                elif k == pygame.K_k:
                    d.sim_shake(t_ms, 1 if mods & pygame.KMOD_SHIFT else 0)
                elif k in (pygame.K_PLUS, pygame.K_EQUALS, pygame.K_KP_PLUS):
                    si = min(2, si + 1)
                    d.sim_timescale(scales[si])
                elif k in (pygame.K_MINUS, pygame.K_KP_MINUS):
                    si = max(0, si - 1)
                    d.sim_timescale(scales[si])
                elif k in (pygame.K_q, pygame.K_a, pygame.K_e, pygame.K_d):
                    manual = True
                    step = 0.15 if k in (pygame.K_q, pygame.K_e) else -0.15
                    if k in (pygame.K_q, pygame.K_a):
                        pl = max(0.2, min(2.6, pl + step))
                    else:
                        pr = max(0.2, min(2.6, pr + step))
                    d.sim_pupil(pl, pr)
                elif k == pygame.K_r:
                    manual = False
                    pl = pr = 1.0
                    d.sim_pupil(-1, -1)
                elif k == pygame.K_s:
                    fn = os.path.join(HERE, "shots", time.strftime("%H%M%S") + ".png")
                    os.makedirs(os.path.dirname(fn), exist_ok=True)
                    round_img(d, 2).save(fn)
                    msg, msg_t = "guardado " + os.path.basename(fn), now
            if ev.type == pygame.MOUSEBUTTONDOWN:
                if ev.button == 1:
                    d.sim_tap(t_ms)
                elif ev.button == 3:
                    petting = True
            if ev.type == pygame.MOUSEBUTTONUP and ev.button == 3:
                petting = False
        if petting:
            d.sim_pet(t_ms, 1)
        else:
            d.sim_pet(t_ms, 0)
        mx, my = pygame.mouse.get_pos()
        if pygame.mouse.get_focused() and my < size:
            gx = (mx - size / 2) / (size / 2) * 1.6
            gy = (my - size / 2) / (size / 2) * 1.6
            d.sim_gaze(gx, gy, t_ms, 2500)
        d.sim_frame(t_ms, dt)
        surf = pygame.surfarray.make_surface(np.ascontiguousarray(frame_rgb(d).swapaxes(0, 1)))
        screen.fill((24, 24, 28))
        screen.blit(pygame.transform.smoothscale(surf, (size, size)), (0, 0))
        screen.blit(mask, (0, 0))
        d.sim_stats(stats)
        x = d.sim_expr()
        sl = d.sim_asleep()
        info = (f"{d.sim_name(x).decode()}{' (zZz)' if sl else ''} | {clock.get_fps():3.0f} fps | "
                f"tiempo x{scales[si]} | USB {'ON' if usb else 'off'}")
        screen.blit(font.render(info, True, (225, 225, 225)), (10, size + 6))
        for i, (lab, col) in enumerate((("energia ", (240, 200, 60)), ("diversion", (90, 190, 240)),
                                        ("cariño  ", (240, 90, 160)))):
            y = size + 30 + i * 18
            screen.blit(small.render(lab, True, (170, 170, 180)), (10, y))
            pygame.draw.rect(screen, (50, 50, 58), (100, y + 2, 200, 10))
            pygame.draw.rect(screen, col, (100, y + 2, int(2 * stats[i]), 10))
        screen.blit(small.render("1-0 C H B M N O expresiones | clic=tocar  clic der=acariciar | G jugar  U USB  K sacudir",
                                 True, (130, 130, 140)), (10, size + 90))
        screen.blit(small.render("ESPACIO siguiente | ENTER color | +/- tiempo | Q/A E/D pupilas | S foto", True,
                                 (130, 130, 140)), (10, size + 105))
        if msg and now - msg_t < 3:
            screen.blit(font.render(msg, True, (120, 220, 120)), (320, size + 30))
        pygame.display.flip()
        clock.tick(60)


if __name__ == "__main__":
    main()
