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

ACELEROMETRO VIRTUAL (M2)  -  alimenta sim_motion() a 60 Hz; panel a la derecha de la ventana
  MODO SENSOR = mantener TAB  (o ` = dejarlo fijo/sin mantener). Mientras esta activo, las letras de abajo son GESTOS y
  NO ejecutan su funcion normal (S foto, F bateria llena, B/N/O expresiones, U USB, W guino, R pupilas, V libre).
  Soltando TAB (o ` otra vez) vuelven todas sus funciones de siempre; no se pierde ninguna tecla anterior.
  TAB + arrastrar con clic izq   inclina el dispositivo (horizontal = balanceo/roll, vertical = cabeceo/pitch; arrastrar
                                 hacia ARRIBA = la pantalla mira al techo). Al soltar vuelve suave a la pose base.
  S sacudida suave   Shift+S sacudida fuerte (senal sintetica ruidosa de ~1 s)   F caida libre + impacto
  V poner de cabeza  B boca abajo  N plano boca arriba  (se quedan hasta pulsar otra pose o R)
  U recogerlo (si no esta plano, primero lo deja plano ~3 s y luego lo levanta)   W caminar (mientras se mantiene)
  R restablecer a "de pie"   O apagar el sensor (sim_motion_off: vuelve al modo sin sensor; cualquier gesto o TAB lo reactiva)
  Panel: flechas 3D, barras ax/ay/az, energia de sacudida (marcas en 0.85 y 2.0 = umbrales suave/fuerte), ultimo evento.
  Sin ventana:  python viewer.py --motion-test [DIR]   (prueba la logica de gestos contra la DLL y guarda panel.png)
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
    d.sim_motion.argtypes = [f32, f32, f32, u32, f32]
    d.sim_motion_state.argtypes = [ctypes.POINTER(ctypes.c_float)]
    d.sim_motion_cool.argtypes = [i32]
    d.sim_motion_cool.restype = u32
    d.sim_clip_active.argtypes = [ctypes.POINTER(ctypes.c_int)]
    d.sim_clip_name.restype = ctypes.c_char_p
    d.sim_clip_name.argtypes = [i32]
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


# ======================================================================= ACELEROMETRO VIRTUAL (M2)
# Convencion de Motion.h (en g): x = derecha de la pantalla, y = ARRIBA, z = hacia el espectador. De pie = (0,+1,0).
PI = float(np.pi)
SENSOR_RANGE = 8.0                       # el sensor satura a +-8 g
MC_NAMES = {0: "sacudida", 1: "caida libre", 2: "impacto", 3: "de cabeza", 4: "recogido", 5: "boca abajo"}
POSE_KEYS = {"upright": (0.0, 0.0), "upside": (PI, 0.0), "facedown": (0.0, -PI / 2), "flat": (0.0, PI / 2)}


def g_from_angles(roll, pitch):
    """vector g (lo que lee el sensor) para un balanceo (+ = lado derecho abajo) y cabeceo (+ = pantalla mira al techo)"""
    return np.array([-np.sin(roll) * np.cos(pitch), np.cos(roll) * np.cos(pitch), np.sin(pitch)])


class VirtualAccel:
    """Acelerometro sintetico. step(dt) devuelve (ax,ay,az) en g. Logica pura (sin pygame) => se puede probar sin ventana.
    Pose base (roll,pitch) suavizada + desplazamiento por arrastre + efectos temporales (sacudida, caida, impacto, caminar)."""
    NOISE = 0.012
    FLAT_NEEDED = 3.0                           # s quieto y plano antes de poder levantarlo (filtros de Motion.h + 1.5 s de mFlatT)

    def __init__(self, seed=5):
        self.rng = np.random.default_rng(seed)
        self.enabled = False
        self.t = 0.0
        self.roll = self.pitch = 0.0            # pose base actual (rad)
        self.roll_t = self.pitch_t = 0.0        # pose base objetivo
        self.off = np.zeros(2)                  # desplazamiento por arrastre (roll, pitch)
        self.off_t = np.zeros(2)
        self.dragging = False
        self.fx = []                            # efectos activos
        self.script = []                        # (t_ejecucion, funcion)
        self.walking = False
        self.pending_lift = False
        self.tau = 0.28                         # constante de tiempo del cambio de pose (s)
        self.flat_for = 0.0                     # segundos continuos en plano
        self.last = np.array([0.0, 1.0, 0.0])
        self.label = "de pie"

    # ---- poses ----
    def set_pose(self, name, label=None):
        self.enabled = True
        self.roll_t, self.pitch_t = POSE_KEYS[name]
        self.label = label or name
        self.script.clear()
        self.pending_lift = False
        self.tau = 0.28

    def reset(self):
        self.set_pose("upright", "de pie")
        self.fx.clear()
        self.off_t[:] = 0

    def drag_to(self, droll, dpitch):
        self.enabled = True
        self.dragging = True
        self.off_t[0] = float(np.clip(droll, -PI, PI))
        self.off_t[1] = float(np.clip(dpitch, -PI / 2, PI / 2))

    def drag_end(self):
        self.dragging = False
        self.off_t[:] = 0                       # retorno suave a la pose base

    # ---- gestos ----
    def shake(self, strong=False):
        self.enabled = True
        ax = self.rng.normal(size=3)
        ax[2] *= 0.5
        ax /= np.linalg.norm(ax)
        self.fx.append(dict(kind="shake", t0=self.t, dur=1.0, amp=4.6 if strong else 1.8, axis=ax,
                            f=float(self.rng.uniform(5.5, 7.0)) if strong else float(self.rng.uniform(4.0, 5.5))))
        self.label = "sacudida fuerte" if strong else "sacudida suave"

    def fall(self, fall_s=0.55):
        self.enabled = True
        self.fx.append(dict(kind="fall", t0=self.t, dur=fall_s + 0.5, fall=fall_s))
        self.label = "caida + impacto"

    def pickup(self):
        """recogerlo: si ya lleva >=1.8 s plano lo levanta ya; si no, lo deja plano ~2 s (Motion.h exige 1.5 s quieto) y luego lo levanta"""
        self.enabled = True
        if self.pitch_t > 1.2 and self.flat_for >= self.FLAT_NEEDED:
            self._lift()
        else:
            self.set_pose("flat", "plano (esperando recogida)")
            self.pending_lift = True

    def _lift(self):
        # OJO: con Motion.h actual el evento "recogido" es casi inalcanzable (ver LEEME_movimiento.md, hallazgo 2)
        self.fx.append(dict(kind="lift", t0=self.t, dur=1.0))
        self.tau = 0.5
        self.roll_t, self.pitch_t = 0.0, 0.0
        self.label = "recogido"

    # ---- un paso de simulacion (llamar a 60 Hz) ----
    def step(self, dt):
        self.t += dt
        for item in [s for s in self.script if s[0] <= self.t]:
            self.script.remove(item)
            item[1]()
        if self.pending_lift and self.flat_for >= self.FLAT_NEEDED:
            self.pending_lift = False
            self._lift()
        k = 1.0 - np.exp(-dt / self.tau)
        self.roll += (self.roll_t - self.roll) * k
        self.pitch += (self.pitch_t - self.pitch) * k
        kd = 1.0 - np.exp(-dt / (0.06 if self.dragging else 0.40))
        self.off += (self.off_t - self.off) * kd
        r, p = self.roll + self.off[0], self.pitch + self.off[1]
        walk_roll = 0.07 * np.sin(2 * PI * 0.95 * self.t) if self.walking else 0.0
        gs = g_from_angles(r + walk_roll, p)
        gscale, add = 1.0, np.zeros(3)
        done = []
        for e in self.fx:
            u = self.t - e["t0"]
            if u >= e["dur"]:
                done.append(e)
                continue
            if e["kind"] == "shake":
                env = np.sin(PI * u / e["dur"]) ** 0.6
                osc = np.sin(2 * PI * e["f"] * u + 1.3 * np.sin(9 * u))
                add += e["axis"] * e["amp"] * env * osc + self.rng.normal(size=3) * 0.18 * e["amp"] * env
            elif e["kind"] == "fall":
                if u < e["fall"]:
                    gscale *= 0.04                       # ingravidez: lectura ~0
                    add += self.rng.normal(size=3) * 0.03
                else:
                    v = u - e["fall"]                    # impacto: pico corto + vibracion amortiguada
                    spike = 3.6 * np.exp(-v / 0.035)
                    ring = 1.4 * np.exp(-v / 0.09) * np.sin(2 * PI * 17 * v)
                    add += np.array([0.35, 1.0, 0.25]) * (spike + ring)
            elif e["kind"] == "lift":
                add += np.array([0.0, 0.0, 0.25 * np.sin(PI * min(u / 0.4, 1.0))]) + self.rng.normal(size=3) * 0.04 * (1 - u)
        for e in done:
            self.fx.remove(e)
        a = gs * gscale + add
        if self.walking:                                 # rebote vertical ~1.9 Hz + vaiven lateral ~0.95 Hz
            bob = 0.22 * np.sin(2 * PI * 1.9 * self.t) + 0.10 * max(0.0, np.sin(2 * PI * 1.9 * self.t + 0.8)) ** 4
            a = a + gs * bob + np.array([0.12 * np.sin(2 * PI * 0.95 * self.t), 0.0, 0.05 * np.sin(2 * PI * 1.9 * self.t)])
        a = a + self.rng.normal(size=3) * self.NOISE
        a = np.clip(a, -SENSOR_RANGE, SENSOR_RANGE)
        settled = abs(self.pitch - PI / 2) < 0.05 and not self.dragging
        self.flat_for = self.flat_for + dt if settled else 0.0
        self.last = a
        return float(a[0]), float(a[1]), float(a[2])


def sensor_key(acc, k, mods):
    """gestos del modo sensor. Devuelve el nombre del gesto o None si la tecla no es de gesto."""
    import pygame
    shift = bool(mods & pygame.KMOD_SHIFT)
    if k == pygame.K_s:
        acc.shake(shift)
        return "sacudida fuerte" if shift else "sacudida suave"
    if k == pygame.K_f:
        acc.fall()
        return "caida libre + impacto"
    if k == pygame.K_v:
        acc.set_pose("upside", "de cabeza")
        return "de cabeza"
    if k == pygame.K_b:
        acc.set_pose("facedown", "boca abajo")
        return "boca abajo"
    if k == pygame.K_n:
        acc.set_pose("flat", "plano boca arriba")
        return "plano boca arriba"
    if k == pygame.K_u:
        acc.pickup()
        return "recoger"
    if k == pygame.K_r:
        acc.reset()
        return "de pie (reset)"
    return None


class MotionMonitor:
    """Detecta el ultimo evento de movimiento disparado por Motion.h comparando las marcas mCool[] (export sim_motion_cool)
    y la bandera mWasUpside (export sim_motion_state). No toca el firmware."""

    def __init__(self, d):
        self.d = d
        self.st = (ctypes.c_float * 16)()
        self.prev = [d.sim_motion_cool(i) for i in range(8)]
        self.was_up = 0.0
        self.log = []                           # [(t_ms, texto)]
        self.ids = (ctypes.c_int * 3)()

    def state(self):
        self.d.sim_motion_state(self.st)
        return self.st

    def poll(self, t_ms):
        st = self.state()
        out = []
        for i in range(6):
            c = self.d.sim_motion_cool(i)
            if c != self.prev[i]:
                self.prev[i] = c
                name = MC_NAMES[i]
                if i == 0:
                    name = "sacudida fuerte" if st[3] > 2.0 else "sacudida suave"
                out.append(name)
        if self.was_up and not st[8]:
            out.append("vuelve a posicion normal")
        self.was_up = st[8]
        for n in out:
            self.log.append((t_ms, n))
        self.log = self.log[-6:]
        return out

    def clips(self):
        self.d.sim_clip_active(self.ids)
        return [self.d.sim_clip_name(self.ids[i]).decode() for i in range(3) if self.ids[i] >= 0]


def rot_to_up(g):
    """rotacion minima que lleva g (ejes del dispositivo) al 'arriba' del mundo (0,1,0): sirve para dibujar el dispositivo"""
    n = np.linalg.norm(g)
    if n < 0.2:
        return np.eye(3)
    a = g / n
    b = np.array([0.0, 1.0, 0.0])
    c = float(a @ b)
    if c < -0.9999:
        return np.diag([-1.0, -1.0, 1.0])
    v = np.cross(a, b)
    K = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    return np.eye(3) + K + K @ K / (1 + c)


def draw_motion_panel(screen, rect, acc, mon, fonts, sensor_mode, latched, now_ms, expr_name):
    """panel del acelerometro virtual (vista 3D del dispositivo + barras + ultimo evento)"""
    import pygame
    font, small = fonts
    x0, y0, w, h = rect
    pygame.draw.rect(screen, (30, 30, 36), rect)
    pygame.draw.line(screen, (60, 60, 70), (x0, y0), (x0, y0 + h))
    st = mon.state()
    on = bool(st[9])
    screen.blit(font.render("ACELEROMETRO " + ("ON" if on else "OFF (sin sensor)"), True,
                            (120, 220, 120) if on else (200, 120, 120)), (x0 + 12, y0 + 8))
    mode = ("modo sensor: " + ("FIJO (`)" if latched else "TAB")) if sensor_mode else "modo sensor: no (TAB o `)"
    screen.blit(small.render(mode, True, (240, 200, 90) if sensor_mode else (130, 130, 140)), (x0 + 12, y0 + 30))
    # ---- vista 3D del dispositivo (mundo fijo, gravedad hacia abajo) ----
    cx, cy, R = x0 + w // 2 - 14, y0 + 120, 62
    g = np.array([st[0], st[1], st[2]]) if on else np.array([0.0, 1.0, 0.0])
    M = rot_to_up(g)

    def proj(p):
        q = M @ p
        return (cx + (q[0] - 0.38 * q[2]) * R, cy - (q[1] + 0.28 * q[2]) * R)

    ang = np.linspace(0, 2 * PI, 40, endpoint=False)
    ring = [np.array([np.cos(a) * 0.95, np.sin(a) * 0.95, 0.0]) for a in ang]
    front = (M @ np.array([0.0, 0.0, 1.0]))[2] >= 0
    pygame.draw.polygon(screen, (60, 66, 84) if front else (36, 38, 48), [proj(p) for p in ring])
    pygame.draw.polygon(screen, (150, 160, 190), [proj(p) for p in ring], 2)
    for ex in (-0.34, 0.34):
        pygame.draw.circle(screen, (240, 210, 90) if front else (90, 80, 40), proj(np.array([ex, 0.12, 0.02])), 7)
    pygame.draw.line(screen, (240, 120, 120), proj(np.array([0, 0.95, 0])), proj(np.array([0, 1.25, 0])), 3)   # marca de ARRIBA de la pantalla
    pygame.draw.line(screen, (110, 170, 240), (cx + 100, cy - 40), (cx + 100, cy + 40), 3)                      # gravedad (mundo)
    pygame.draw.polygon(screen, (110, 170, 240), [(cx + 100, cy + 52), (cx + 92, cy + 38), (cx + 108, cy + 38)])
    screen.blit(small.render("grav", True, (110, 170, 240)), (cx + 84, cy + 54))
    # ---- barras ax ay az + energia de sacudida ----
    by = y0 + 200
    a = acc.last if on else np.zeros(3)
    for i, (lab, col) in enumerate((("ax", (240, 110, 110)), ("ay", (110, 220, 130)), ("az", (110, 160, 250)))):
        yy = by + i * 18
        v = float(a[i])
        screen.blit(small.render(f"{lab} {v:+5.2f}", True, (200, 200, 205)), (x0 + 12, yy))
        bx, bw = x0 + 100, 150
        pygame.draw.rect(screen, (50, 50, 58), (bx, yy + 2, bw, 10))
        pygame.draw.line(screen, (120, 120, 130), (bx + bw // 2, yy), (bx + bw // 2, yy + 14))
        L = int(np.clip(v / 4.0, -1, 1) * bw / 2)
        pygame.draw.rect(screen, col, (bx + bw // 2 + min(L, 0), yy + 2, abs(L), 10))
    yy = by + 3 * 18
    sh = st[3] if on else 0.0
    screen.blit(small.render(f"sacud {sh:4.2f}", True, (200, 200, 205)), (x0 + 12, yy))
    pygame.draw.rect(screen, (50, 50, 58), (x0 + 100, yy + 2, 150, 10))
    pygame.draw.rect(screen, (240, 170, 60) if sh < 2.0 else (240, 80, 80), (x0 + 100, yy + 2, int(min(sh / 4.0, 1) * 150), 10))
    for thr in (0.85, 2.0):
        tx = x0 + 100 + int(thr / 4.0 * 150)
        pygame.draw.line(screen, (230, 230, 235), (tx, yy), (tx, yy + 14))
    mag = float(np.linalg.norm(a))
    screen.blit(small.render(f"|a| {mag:4.2f} g  pose: {acc.label}", True, (170, 170, 180)), (x0 + 12, yy + 20))
    flags = []
    if on and st[7]:
        flags.append("CAYENDO")
    if on and st[8]:
        flags.append("de cabeza")
    if on and st[2] > 0.85:
        flags.append("plano")
    if on and st[2] < -0.8:
        flags.append("boca abajo")
    if acc.walking:
        flags.append("caminando")
    screen.blit(small.render("estado: " + (", ".join(flags) or "-"), True, (170, 170, 180)), (x0 + 12, yy + 38))
    # ---- ultimo evento ----
    ey = yy + 66
    screen.blit(font.render("ULTIMO EVENTO", True, (225, 225, 225)), (x0 + 12, ey))
    if mon.log:
        t_ev, nm = mon.log[-1]
        age = (now_ms - t_ev) / 1000.0
        fresh = max(0.0, 1 - age / 4.0)
        col = (int(120 + 135 * fresh), int(200 + 40 * fresh), int(120 + 30 * fresh))
        screen.blit(font.render(nm, True, col), (x0 + 12, ey + 22))
        screen.blit(small.render(f"hace {age:4.1f} s", True, (150, 150, 160)), (x0 + 175, ey + 24))
        for i, (tt, n2) in enumerate(reversed(mon.log[:-1][-4:])):
            screen.blit(small.render(f"{(now_ms - tt) / 1000.0:5.1f}s  {n2}", True, (130, 130, 140)), (x0 + 12, ey + 46 + i * 15))
    else:
        screen.blit(small.render("(ninguno todavia)", True, (130, 130, 140)), (x0 + 12, ey + 24))
    screen.blit(small.render(f"expr: {expr_name}", True, (170, 170, 180)), (x0 + 12, ey + 112))
    screen.blit(small.render("clips: " + (", ".join(mon.clips()) or "-"), True, (170, 170, 180)), (x0 + 12, ey + 128))
    keys = ["TAB(mantener)/` = modo sensor", "TAB+arrastrar inclina  O sensor off",
            "S suave  Shift+S fuerte  F caida", "V cabeza  B boca abajo  N plano", "U recoger  W caminar  R de pie"]
    for i, ln in enumerate(keys):
        screen.blit(small.render(ln, True, (120, 120, 130)), (x0 + 12, y0 + h - 84 + i * 15))


def motion_test(outdir):
    """prueba sin ventana: simula senales a 60 Hz contra la DLL, imprime los eventos detectados y guarda panel.png"""
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    import pygame
    pygame.init()
    d = load()
    clock = {"t": 0}

    def feed(acc, mon, evs=None):
        ax, ay, az = acc.step(1 / 60)
        clock["t"] += 17
        d.sim_motion(ax, ay, az, clock["t"], 1 / 60)
        d.sim_frame(clock["t"], 1 / 60)
        for n in mon.poll(clock["t"]):
            if evs is not None:
                evs.append((clock["t"], n))

    def scenario(name, setup, secs, walk=False):
        d.sim_init(4242)
        clock["t"] = run_frames(d, 0, 300)
        acc = VirtualAccel(seed=3)
        mon = MotionMonitor(d)
        acc.enabled = True
        for _ in range(60):
            feed(acc, mon)                       # 1 s de reposo para que el filtro de gravedad se asiente
        setup(acc)
        acc.walking = walk
        evs, t_start = [], clock["t"]
        for _ in range(int(secs * 60)):
            feed(acc, mon, evs)
        shown = [(round((e[0] - t_start) / 1000, 2), e[1]) for e in evs] or "ningun evento"
        print(f"{name:30s} -> {shown} | expr={d.sim_name(d.sim_expr()).decode()} clips={mon.clips()}")
        return acc, mon

    scenario("reposo de pie (5 s)", lambda a: a.reset(), 5)
    scenario("sacudida suave", lambda a: a.shake(False), 2)
    scenario("sacudida fuerte", lambda a: a.shake(True), 2)
    scenario("caida libre + impacto", lambda a: a.fall(), 2.5)
    scenario("de cabeza (V)", lambda a: a.set_pose("upside"), 4)

    def up_then_back(a):
        a.set_pose("upside")
        a.script.append((a.t + 3.0, a.reset))
    scenario("cabeza 3 s -> de pie", up_then_back, 6)
    scenario("boca abajo (B)", lambda a: a.set_pose("facedown"), 3)
    scenario("recoger (U) desde de pie", lambda a: a.pickup(), 10)
    scenario("plano (N) 3 s", lambda a: a.set_pose("flat"), 3)
    scenario("caminar (W) 6 s", lambda a: a.reset(), 6, walk=True)
    acc, mon = scenario("arrastre inclinado", lambda a: a.drag_to(0.6, 0.3), 1.5)
    screen = pygame.Surface((270, 840))
    screen.fill((24, 24, 28))
    draw_motion_panel(screen, (0, 0, 270, 840), acc, mon,
                      (pygame.font.SysFont("consolas", 16), pygame.font.SysFont("consolas", 13)),
                      True, False, clock["t"], d.sim_name(d.sim_expr()).decode())
    os.makedirs(outdir, exist_ok=True)
    fn = os.path.join(outdir, "panel.png")
    pygame.image.save(screen, fn)
    print("guardado", fn)
    acc.drag_end()
    for _ in range(120):
        acc.step(1 / 60)
    print("2 s tras soltar, desplazamiento de arrastre =", np.round(acc.off, 3))
    d.sim_motion_off()
    print("sensor off -> mActive =", int(mon.state()[9]))


def main():
    if "--motion-test" in sys.argv:
        i = sys.argv.index("--motion-test")
        motion_test(sys.argv[i + 1] if i + 1 < len(sys.argv) and not sys.argv[i + 1].startswith("--") else os.path.join(HERE, "shots"))
        return
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
    HUD = 140
    PANEL = 270
    screen = pygame.display.set_mode((size + PANEL, size + HUD))
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
    acc = VirtualAccel(seed=int(time.time()) & 0xFFFF)
    mon = MotionMonitor(d)
    tab_held = latched = False
    drag0 = None
    acc_t = 0.0
    while True:
        now = time.time()
        t_ms = int((now - t0) * 1000)
        dt = min(now - lastt, 0.04)
        lastt = now
        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                return
            if ev.type == pygame.KEYUP and ev.key == pygame.K_TAB:
                tab_held = False
            if ev.type == pygame.KEYDOWN:
                k = ev.key
                name = pygame.key.name(k)
                mods = pygame.key.get_mods()
                if k == pygame.K_TAB:
                    tab_held = True
                    acc.enabled = True
                    continue
                if k == pygame.K_BACKQUOTE:
                    latched = not latched
                    acc.enabled = acc.enabled or latched
                    continue
                if (tab_held or latched) and k != pygame.K_ESCAPE:
                    if k == pygame.K_o:
                        acc.enabled = False
                        d.sim_motion_off()
                        msg, msg_t = "sensor OFF (modo sin sensor)", now
                        continue
                    gname = sensor_key(acc, k, mods)
                    if gname:
                        msg, msg_t = "gesto: " + gname, now
                        continue
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
                if ev.button == 1 and (tab_held or latched) and ev.pos[0] < size and ev.pos[1] < size:
                    drag0 = ev.pos
                    acc.drag_to(0.0, 0.0)
                elif ev.button == 1:
                    d.sim_tap(t_ms)
                elif ev.button == 3:
                    petting = True
            if ev.type == pygame.MOUSEBUTTONUP and ev.button == 3:
                petting = False
            if ev.type == pygame.MOUSEBUTTONUP and ev.button == 1 and drag0 is not None:
                drag0 = None
                acc.drag_end()
            if ev.type == pygame.MOUSEMOTION and drag0 is not None:
                acc.drag_to((ev.pos[0] - drag0[0]) / size * 1.5 * np.pi, -(ev.pos[1] - drag0[1]) / size * np.pi)
        if petting:
            d.sim_pet(t_ms, 1)
        else:
            d.sim_pet(t_ms, 0)
        mx, my = pygame.mouse.get_pos()
        sensor_mode = tab_held or latched
        acc.walking = sensor_mode and pygame.key.get_pressed()[pygame.K_w]
        if acc.enabled:                     # 60 Hz fijos, independientes de los fps de la ventana
            acc_t += dt
            n = 0
            while acc_t >= 1 / 60 and n < 4:
                acc_t -= 1 / 60
                n += 1
                ax_, ay_, az_ = acc.step(1 / 60)
                d.sim_motion(ax_, ay_, az_, t_ms, 1 / 60)
        else:
            acc_t = 0.0
        if pygame.mouse.get_focused() and my < size and mx < size and drag0 is None:
            gx = (mx - size / 2) / (size / 2) * 1.6
            gy = (my - size / 2) / (size / 2) * 1.6
            d.sim_gaze(gx, gy, t_ms, 2500)
        d.sim_frame(t_ms, dt)
        mon.poll(t_ms)
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
        screen.blit(small.render("SENSOR: TAB(mantener)/` + arrastrar inclina | S F V B N U W R O = gestos (ver panel)", True,
                                 (240, 200, 90) if sensor_mode else (130, 130, 140)), (10, size + 120))
        draw_motion_panel(screen, (size, 0, PANEL, size + HUD), acc, mon, (font, small), sensor_mode, latched, t_ms,
                          d.sim_name(x).decode())
        if msg and now - msg_t < 3:
            screen.blit(font.render(msg, True, (120, 220, 120)), (320, size + 30))
        pygame.display.flip()
        clock.tick(60)


if __name__ == "__main__":
    main()
