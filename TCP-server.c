#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct header {
  char source_port_address[16];
  char destination_port_address[16];
  char sequence_number[32];
  char acknowledgment_number[32];
  char HLEN[4];
  char reserved[6];
  char type[7];
  char window_size[16];
  char checksum[16];
  char urgent_pointer[16];
};

struct segment {
  struct header head;
  char data[1024];
};

void initializeHeader(struct header *hdr, char *random_str) {
  strcpy(hdr->source_port_address, "54321");
  strcpy(hdr->destination_port_address, "12345");
  strcpy(hdr->sequence_number, random_str);
  strcpy(hdr->acknowledgment_number, "0");
  strcpy(hdr->HLEN, "5");
  strcpy(hdr->reserved, "000000");
  strncpy(hdr->type, "000010", sizeof(hdr->type) - 1);
  hdr->type[sizeof(hdr->type) - 1] = '\0';
  strcpy(hdr->window_size, "8192");
  strcpy(hdr->checksum, "0000");
  strcpy(hdr->urgent_pointer, "0000");
}

int main(int argc, char *argv[]) {
  srand(time(0));
  char random_number_str[4]; // At least 3 digits for a random number
  snprintf(random_number_str, sizeof(random_number_str), "%d", rand() % 1000);

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int Port = atoi(argv[1]);

  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket Creation Failed!");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(Port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    perror("Error binding socket");
    exit(1);
  }

  if (listen(server_socket, 5) == -1) {
    perror("Error listening for connections");
    exit(1);
  }

  printf("Server listening on port %d...\n", Port);

  client_socket =
      accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    perror("Error accepting connection");
    exit(1);
  }

  printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  struct segment receivedSegment;

  // Initialization for 3-way handshake
  ssize_t bytes_received =
      read(client_socket, &receivedSegment, sizeof(struct segment));
  if (bytes_received <= 0) {
    printf("Client disconnected.\n");
  }
  printf("\nReceived:\nSource Port - %s\nDestination Port - %s\nSequence "
         "Number - %s\nControl Field - %s\n\n",
         receivedSegment.head.source_port_address,
         receivedSegment.head.destination_port_address,
         receivedSegment.head.sequence_number, receivedSegment.head.type);

  strncpy(receivedSegment.head.type, "010010",
          sizeof(receivedSegment.head.type) - 1);
  receivedSegment.head.type[sizeof(receivedSegment.head.type) - 1] = '\0';

  int temp = atoi(receivedSegment.head.sequence_number) + 1;
  char temp2[50];
  snprintf(temp2, sizeof(temp2), "%d", temp);
  strcpy(receivedSegment.head.acknowledgment_number, temp2);
  strcpy(receivedSegment.head.sequence_number, random_number_str);

  // Prompt for input and send it back to the client
  printf("Enter a window size to send back to the client: ");
  fgets(receivedSegment.head.window_size,
        sizeof(receivedSegment.head.window_size), stdin);
  receivedSegment.head
      .window_size[strcspn(receivedSegment.head.window_size, "\n")] = '\0';

  // Send the entire structure (header + data)
  write(client_socket, &receivedSegment, sizeof(struct segment));

  bytes_received =
      read(client_socket, &receivedSegment, sizeof(struct segment));
  if (bytes_received <= 0) {
    printf("Client disconnected.\n");
  }
  printf("\nReceived:\nSource Port - %s\nDestination Port - %s\nAcknowledgment "
         "Number - %s\nControl Field - %s\n\n",
         receivedSegment.head.source_port_address,
         receivedSegment.head.destination_port_address,
         receivedSegment.head.acknowledgment_number, receivedSegment.head.type);

  // Continuous communication
  printf("Connection Established...Data Transfer Begins...\n\n");
  while (1) {
    ssize_t bytes_received =
        read(client_socket, &receivedSegment, sizeof(struct segment));
    if (bytes_received <= 0) {
      printf("Client disconnected.\n");
      break;
    }

    printf("\nReceived:\nSource Port - %s\nDestination Port - %s\nSequence"
           " Number - %s\nData - %s\n\n",
           receivedSegment.head.source_port_address,
           receivedSegment.head.destination_port_address,
           receivedSegment.head.sequence_number, receivedSegment.data);

    // Check for the termination condition
    if (strcmp(receivedSegment.data, "exit") == 0) {
      printf("Received 'exit' from client. Terminating server.\n");
      break;
    }

    // Update acknowledgment
    int new_ack = atoi(receivedSegment.head.sequence_number) +
                  (strlen(receivedSegment.data) * 50);
    char temp[50];
    snprintf(temp, sizeof(temp), "%d", new_ack);
    strcpy(receivedSegment.head.acknowledgment_number, temp);
    // printf("Data  = %s\n",receivedSegment.data);
    // printf("New ack = %s\n",receivedSegment.head.acknowledgment_number);

    // Prompt for input and send it back to the client
    printf("Enter a window size to send back to the client: ");
    fgets(receivedSegment.head.window_size,
          sizeof(receivedSegment.head.window_size), stdin);
    receivedSegment.head
        .window_size[strcspn(receivedSegment.head.window_size, "\n")] = '\0';

    // Send the entire structure (header + data)
    write(client_socket, &receivedSegment, sizeof(struct segment));
  }

  close(client_socket);
  close(server_socket);

  return 0;
}
