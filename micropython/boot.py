# code='''

def ls():
  import os
  print(os.listdir())

def reset():
  import machine
  machine.reset()

def mem():
  import micropython
  micropython.mem_info()

def cat(filename):
  file = open(filename, "r")
  while True:
    data=file.readline()
    if data=='':
      break
    print(data.rstrip("\n"))
  file.close()
  
def delete(filename):
  file = open(filename, "w")
  file.close()
  
# with open('boot.py','w') as f: f.write(code) '''
