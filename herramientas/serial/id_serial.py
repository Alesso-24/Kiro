import serial,time
s=serial.Serial('COM6',115200,timeout=1);time.sleep(2.2);s.write(b'?')
t=time.time();ident=None;rows=[]
while time.time()-t<7:
    l=s.readline().decode(errors='ignore').strip()
    if '[ID]' in l: ident=l
    if 'PERF' in l: rows.append(l.split('filas')[-1].strip())
print(ident);print('filas por cuadro (si no nace deben ser ~23-30):',rows)
