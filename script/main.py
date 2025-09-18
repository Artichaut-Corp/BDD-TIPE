
import socket

HOST = "127.0.0.1"  # Adresse du serveur
PORT = 8080         # Même port que le serveur

# Créer le socket
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connexion
client.connect((HOST, PORT))
print("Connecté au serveur C++")

try:
    while True:
        msg = input("Message à envoyer: ")
        if msg.lower() == "quit":
            break
        client.sendall(msg.encode("utf-8"))

        data = client.recv(1024)
        print("Réponse du serveur:", data.decode("utf-8"))
finally:
    client.close()

