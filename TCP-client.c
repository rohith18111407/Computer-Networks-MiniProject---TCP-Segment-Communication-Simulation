#include <arpa/inet.h>
#include <netinet/in.h>
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
  strcpy(hdr->source_port_address, "12345");
  strcpy(hdr->destination_port_address, "54321");
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

  int client_socket;
  struct sockaddr_in server_addr;

  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    perror("Socket Creation Failed!");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(Port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("Socket Connect Failed!");
    exit(1);
  }

  struct segment tcpSegment;
  initializeHeader(&tcpSegment.head, random_number_str);

  // Initialization
  // Send the entire structure (header + data)
  write(client_socket, &tcpSegment, sizeof(struct segment));

  ssize_t bytes_received =
      read(client_socket, &tcpSegment, sizeof(struct segment));
  if (bytes_received <= 0) {
    printf("Client disconnected.\n");
  }
  char new_seq[50];
  strcpy(new_seq, tcpSegment.head.acknowledgment_number);
  printf(
      "\nReceived:\nSource Port - %s\nDestination Port - %s\nSequence Number "
      "- %s\nAcknowledgment Number - %s\nControl Field - %s\nWindow Size - "
      "%s\n\n",
      tcpSegment.head.source_port_address,
      tcpSegment.head.destination_port_address, tcpSegment.head.sequence_number,
      tcpSegment.head.acknowledgment_number, tcpSegment.head.type,
      tcpSegment.head.window_size);

  int ackno = atoi(tcpSegment.head.sequence_number) + 1;
  char temp2[50];
  snprintf(temp2, sizeof(temp2), "%d", ackno);
  strcpy(tcpSegment.head.acknowledgment_number, temp2);
  strcpy(tcpSegment.head.type,"010000");

  // Send the entire structure (header + data)
  write(client_socket, &tcpSegment, sizeof(struct segment));

  // Continuous communication
  printf("Connection Established...Data Transfer Begins...\n\n");
  strcpy(tcpSegment.head.sequence_number, new_seq);
  while (1) {
    printf("Enter a message : ");
    fgets(tcpSegment.data, sizeof(tcpSegment.data), stdin);
    tcpSegment.data[strcspn(tcpSegment.data, "\n")] = '\0';

    // Check for the exit condition
    if (strcmp(tcpSegment.data, "exit") == 0) {
      printf("Terminating client.\n");
      break;
    }

    // Send the entire structure (header + data)
    write(client_socket, &tcpSegment, sizeof(struct segment));

    // Now, wait for the server's response
    ssize_t bytes_received =
        read(client_socket, &tcpSegment, sizeof(struct segment));
    if (bytes_received <= 0) {
      printf("Client disconnected.\n");
      break;
    }

    printf("\nReceived:\nSource Port - %s\nDestination Port - "
           "%s\nAcknowledgment Number - %s\nWindow"
           " Size - %s\n\n",
           tcpSegment.head.source_port_address,
           tcpSegment.head.destination_port_address,
           tcpSegment.head.acknowledgment_number, tcpSegment.head.window_size);

    strcpy(tcpSegment.head.sequence_number,
           tcpSegment.head.acknowledgment_number);
  }

  close(client_socket);

  return 0;
}
