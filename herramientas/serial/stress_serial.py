import serial,time,sys
s=serial.Serial("COM6",115200,timeout=0.3)
t0=time.time(); last_line=time.time(); n=0; mins=[]; sent=0; nextsend=0
keys=b" jkK v x g t p w n c c n c 1 2 3 4 5 6 7 8 9"
ki=0
while time.time()-t0<120:
    if time.time()>=nextsend:
        s.write(bytes([keys[ki%len(keys)]])); ki+=1; nextsend=time.time()+1.6
    l=s.readline().decode(errors='ignore').strip()
    if "PERF" in l:
        n+=1; last_line=time.time()
        try: mins.append(int(l.split()[1]))
        except: pass
        if n%8==0: print(l)
    if time.time()-last_line>4:
        print("!! SIN PERF por 4 s -> posible cuelgue en t=%.0f"%(time.time()-t0)); break
print("lineas PERF:",n,"fps min/prom:",min(mins) if mins else None, sum(mins)/len(mins) if mins else None)
s.close()
