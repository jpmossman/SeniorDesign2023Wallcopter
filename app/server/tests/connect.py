import socket

s = socket.socket()         

s.bind(('172.20.10.3', 8000 ))
s.listen(0)
print("listening...")
while True:
    print("listening...")
    client, addr = s.accept()

    while True:
        content = client.recv(32)
        if len(content) ==0:
           break
        else:
            print(content)

    print("Closing connection")

    client.close()
