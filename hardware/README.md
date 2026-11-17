# hardware/

## `pcb_altium/` — COPIA del proyecto de Altium
Copia de `C:\Users\Public\Documents\Altium\PCB_TAMAGOCHI_FINAL\` (proyecto **Altium 365 / Git administrado**, Altium Designer 26.10.1) hecha el 2026-11-17. **No se movió el original**: el proyecto está ligado a su servidor de Altium (`.git` con remoto en `afs-vcs-ue1.365.altium.com`) y a rutas absolutas, y moverlo lo rompería. Esta copia **excluye la carpeta `.git`** del proyecto original para no anidar repositorios.

| Archivo | Qué es |
|---|---|
| `PCB_TAMAGOCHI_FINAL.PrjPcb` | Proyecto de Altium |
| `Sheet1.SchDoc` | Esquemático (única hoja) |
| `PCB1.PcbDoc` | **PCB enviada a fabricar** (31-oct) |
| `PCB_ALTIUM_PRUEBA.PcbDoc` | Placa de práctica / tutorial (10-nov) |
| `CAMtastic1.Cam`, `CAMtastic2.Cam` | Documentos CAM (revisión de Gerbers) |
| `Project Outputs for PCB_TAMAGOCHI_FINAL/` | Gerbers, NC Drill, Pick & Place, BOM, informes DRC (salidas de fabricación de `PCB1`) |
| `Project Logs for PCB_TAMAGOCHI_FINAL/` | Logs de ECO (importar cambios) |
| `History/` | Versiones anteriores (zip) — excluido de git (`.gitignore`) |
| `__Previews/` | Vista previa del esquemático |

> Para editar el diseño usa **el proyecto original** en `C:\Users\Public\Documents\Altium\PCB_TAMAGOCHI_FINAL\` (sincronizado con Altium 365). Esta copia sirve de respaldo, para consulta y para que la documentación enlace a los archivos reales. Si se quiere versionar el cambio, copiar de vuelta los archivos modificados.

## `fabricacion/`
- `Gerbers_Alux.zip` — paquete de fabricación del 31-oct-2026 (el nombre «Alux» es el nombre anterior; la placa es hoy **Kiro v1**).
- `2026-10-23 08_46 From Parts Cart JLCPCB  Parts.xlsx` — carrito de partes exportado.
- `PCB_TAMAGOCHI_FINAL.BomDoc` — documento BOM de Altium.

Ver [docs/05-fabricacion.md](../docs/05-fabricacion.md).
