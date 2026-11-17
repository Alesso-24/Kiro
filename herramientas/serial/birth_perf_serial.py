import serial,time
s=serial.Serial('COM6',115200,timeout=1);time.sleep(2.2);s.write(b'n')
t=time.time();out=[]
while time.time()-t<9: 
    l=s.readline().decode(errors='ignore').strip()
    if 'PERF' in l: out.append(l)
print('\n'.join(out))
