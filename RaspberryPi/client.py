import socket
import sys

server = socket.socket()
host = '192.168.2.6'
port = 9876

def newPointsData(x, y, z):
	return ('X coord:' + x + ' Y coord:' + y + ' Z coord:' + z)

server.connect((host, port))

server.sendall(newPointsData(sys.argv[1], sys.argv[2], sys.argv[3]).encode())
print(server.recv(1024).decode()) #Normal server response
server.sendall('Quit'.encode())
print(server.recv(1024).decode()) #Quit server response

server.close()
