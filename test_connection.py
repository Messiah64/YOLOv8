import socket
import ujson

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Host and port for the server
HOST = '0.0.0.0'  # Listen on all available interfaces
PORT = 12345

# Bind the socket to the host and port
server_socket.bind((HOST, PORT))

# Listen for incoming connections
server_socket.listen(1)
print('Server listening on', PORT)

while True:
    # Accept incoming connection
    client_socket, client_address = server_socket.accept()
    print('Connected by', client_address)

    # Example array data
    data = [1, 2, 3, 4, 5,6,7]

    # Convert array to JSON
    json_data = ujson.dumps(data)

    try:
        # Send JSON data to the client
        client_socket.sendall(json_data.encode())
        print('Data sent successfully')
    except Exception as e:
        print('Error sending data:', e)

    # Close the connection
    client_socket.close()
