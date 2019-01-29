
osn=40320

a=0
c=0
cy=0
a= 66
c+=1
print("In lin")
while True:
  x=0
  while c<10:
    b = a * osn + osn - 1
    if b>18446744073709551615:
      break
    x+=1
    a=b
  c+=x
  if x>0:
    print("In", x)
  if a==0:
    break
  cy+=1
  a = a // 256
  print("Out")

print("Out Sum: ",cy)
