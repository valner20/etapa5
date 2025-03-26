#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *receive_messages(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        printf("\n%s\n", buffer);
    }
    return NULL;
}

void start_client(const char *ip, int port) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor en %s:%d\n", ip, port);

    // Solicitar el nombre del usuario
    char buffer[1024];
    printf("Ingrese su nombre: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Eliminar el salto de línea que fgets siempre incluye
    send(client_socket, buffer, strlen(buffer), 0);

    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, &client_socket);
    pthread_detach(receive_thread);

    while (1) {
        printf("Escribe un mensaje: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (strncmp(buffer, "exit", 4) == 0) { //compara los primeros 4 caracteres con exit, si es exit se desconecta el cliente
            break;
        }
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
    }

    close(client_socket);
}

int main() {

    start_client("192.168.1.166", 8080);
    return 0;
}