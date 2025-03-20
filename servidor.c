#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 3

typedef struct {
    int socket;
    struct sockaddr_in address;
    char name[50]; // Agregamos un campo para el nombre del cliente
} client_t;

client_t *clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i]->socket != sender_socket) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}



void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[1024];

    // Recibe el nombre del cliente
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client->socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        close(client->socket);
        free(client);
        return NULL;
    }
    strncpy(client->name, buffer, sizeof(client->name) - 1); // Guarda el nombre del cliente
    client->name[sizeof(client->name) - 1] = '\0'; // Asegura terminación nula

    printf("Cliente conectado con IP %s y nombre %s\n",
           inet_ntoa(client->address.sin_addr), client->name);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client->socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            close(client->socket);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < num_clients; i++) {
                if (clients[i]->socket == client->socket) {
                    clients[i] = clients[num_clients - 1];
                    num_clients--;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            free(client);
            break;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client->address.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Mensaje recibido de %s (%s): %s\n", client->name, client_ip, buffer);

        // Preparar el mensaje para otros clientes
        char message[1074];
        snprintf(message, sizeof(message), "%s: %s", client->name, buffer);

        broadcast_message(message, client->socket);
    }

    return NULL;
}

void start_server(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.1.162");
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar el socket del servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error al escuchar en el socket del servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en:%d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients < MAX_CLIENTS) {
            client_t *client = malloc(sizeof(client_t));
            client->socket = client_socket;
            client->address = client_addr;
            clients[num_clients++] = client;
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handle_client, client);
            pthread_detach(client_thread);
        } else {
            close(client_socket);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

int main() {
    start_server(12346);
    return 0;
}
