import socket

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Server address and port
HOST = '127.0.0.1'  # Server IP address (localhost in this case)
PORT = 12345

# Connect to the server
client_socket.connect((HOST, PORT))
print('Connected to server')

# Wait for data to be received
while True:
    data = client_socket.recv(1024)
    if not data:
        break
    print('Received data:', data.decode())

# Close the socket
client_socket.close()
