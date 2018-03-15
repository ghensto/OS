
#include "packet.h"

int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;             /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */

/*
   Handles the incoming packet.
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order.
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
   Hint: "which" field in the packet will be useful.
 */
static void packet_handler(int sig) {

  // Ignore any SIGIO that occurs while in signal handler other signals
  signal(SIGIO, SIG_IGN);

  packet_t pkt;
  void *chunk;

  int bytesRead = 0;

  // TODO get the "packet_queue_msg" from the queue.
  packet_queue_msg pkt_message;
  bytesRead = msgrcv(msqid, &pkt_message, sizeof(pkt_message.pkt), QUEUE_MSG_TYPE, 0);

  if (bytesRead == -1) {
    perror("Failed to receive packet from packet_sender.\n");
    exit(-1);
  }

  // TODO extract the packet from "packet_queue_msg" and store it in the memory from memory manager
  //pkt_message.pkt
  packet_t received_packet = pkt_message.pkt;

  pkt_cnt += 1;
  pkt_total = received_packet.how_many;
  message.num_packets = received_packet.how_many;

  // Get the pointer to a memory chunk; put this in the data array in the correct order
  // according to the packet's 'which' property
  message.data[received_packet.which] = mm_get(&mm);
  memcpy(message.data[received_packet.which], received_packet.data, sizeof(received_packet.data));

  // Restore signal
  signal(SIGIO, packet_handler);

}

/*
 * TODO - Create message from packets ... deallocate packets.
 * Return a pointer to the message on success, or NULL
 */
static char *assemble_message() {

  int msg_index = 0;
  char *msg;
  int msg_len = message.num_packets * sizeof(data_t);
  msg = (char *) malloc(msg_len+1);

  /* TODO - Allocate msg and assemble packets into it */
  for (int pkt_index = 0; pkt_index < pkt_total; pkt_index++) {
      memcpy(msg + msg_index, message.data[pkt_index], sizeof(message.data[pkt_index]));
      msg_index += strlen(message.data[pkt_index]);
  }

  /* reset these for next message */
  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;
  msg_index = 0;

  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to receive>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* no of messages you will get from the sender */
  int i;
  char *msg;

  /* TODO - init memory manager for NUM_CHUNKS chunks of size CHUNK_SIZE each */
  if (mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE) == -1) { 
      perror("Failed to allocate memory for messages.\n");
      exit(-1);
  }

  message.num_packets = 0;

  /* TODO initialize msqid to send pid and receive messages from the message queue. Use the key in packet.h */
  if ((msqid = msgget(key, 0666| IPC_CREAT)) == -1) {
    perror("Failed to create new message queue in receiver.\n");
    exit(-1);
  }

  /* TODO send process pid to the sender on the queue */
  pid_queue_msg pid_message;
  pid_message.mtype = QUEUE_MSG_TYPE;
  pid_message.pid = getpid();
  printf("Receiver pid is %d\n", pid_message.pid);
  if (msgsnd(msqid, &pid_message, sizeof(pid_message.pid), 0) == -1) {
    perror("Failed to send pid message.\n");
    exit(-1);
  }

  /* TODO set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/
  struct sigaction pkt_action;
  pkt_action.sa_handler = packet_handler;
  pkt_action.sa_flags = 0;

  if ((sigfillset(&pkt_action.sa_mask) == -1) ||
      (sigaction(SIGIO, &pkt_action, NULL) == -1)) {
    perror("Failed to set SIGIO signal handler.");
    exit(-1);
  }

  // For each message
  for (i = 1; i <= k; i++) {
    // For each packet in a message
    while (pkt_cnt < pkt_total) {
      pause(); /* block until next packet */
    }

    msg = assemble_message();
    if (msg == NULL) {
      perror("Failed to assemble message");
    }
    else {
      fprintf(stderr, "GOT IT: message=%s\n", msg);
      free(msg);
    }
  }

  // TODO deallocate memory manager
  mm_release(&mm);

  // TODO remove the queue once done
  msgctl(msqid, IPC_RMID, NULL);

  return EXIT_SUCCESS;
}
