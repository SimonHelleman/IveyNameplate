import socket
import select

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        client_socket.connect(("127.0.0.1", 25565))
    except TimeoutError as e:
        print("timeout")
        raise e
    
    client_socket.setblocking(False)  # Set socket to non-blocking mode
    
    while True:
        try:
            readable, _, _ = select.select([client_socket], [], [], 1.0) # timeout 1 second

            if readable:
                buf = client_socket.recv(1024)
                if buf:
                    print(buf.decode())
                    client_socket.send(buf)
        except KeyboardInterrupt:
            client_socket.close()
            break
    print("end")