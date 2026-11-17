NL=bytes([10]); BS=bytes([92]); Q=bytes([34]); CR=bytes([13])
d=open('aud.cpp','rb').read().replace(CR+NL,NL)
L=d.split(NL); out=[]; i=0
while i<len(L):
    s=L[i]
    # linea con printf( y numero impar de comillas (sin contar \" )
    t=s.replace(BS+Q,b'')
    if b'printf(' in t and t.count(Q)%2==1 and i+1<len(L) and L[i+1].lstrip().startswith(Q):
        out.append(s+BS+b'n'+L[i+1].lstrip()); i+=2; continue
    out.append(s); i+=1
open('aud.cpp','wb').write(NL.join(out))
