
#include <vector>
#include <string>
#include <map>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <signal.h>
#include <set>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


using namespace std;

#define BUFFER_SIZE 512
#define PORT 8080

struct sockaddr_in server, from;
int sock;
int one = 1;
int drop_rate = 0.1;

unsigned int WINDOW_SIZE;

int BASE;

bool received_packets[20000];
bool sent_ack[20000];
bool should_drop_this[20000];

bool all_received;
bool all_sent_ack;
bool without_drop;

int NUMBER_OF_PACKETS;
int expected_packet;


set <int> S;
queue <int> ack_array;

socklen_t slen, recv_len;


typedef struct packet1 {
	int seq_no;
} ACK_PKT;

typedef struct packet2 {
	int seq_no;
	char data[BUFFER_SIZE];
	int packet_size;
} DATA_PKT;

DATA_PKT rcv_pkt;
ACK_PKT ack_pkt;
queue < packet2 > packetQuere;
map < int, packet2 > ALL_DATA;


void dont_drop() {
	 for(int i = 0; i < NUMBER_OF_PACKETS+5; i++){
		should_drop_this[i] = false;
	}
}

void* receiving(void* one) {
	int NN = 0;
	expected_packet = 0;
	BASE = 0;
	while (!all_received) {
		all_received = true;
		fflush(stdout);

		recv_len = recvfrom(sock, &rcv_pkt, BUFFER_SIZE, 0, (struct sockaddr *) &from, &slen);

		while (received_packets[expected_packet] == true && (int)packetQuere.size() <= WINDOW_SIZE) {
				packetQuere.pop();
				BASE++;
				expected_packet++;
		}

		if (received_packets[rcv_pkt.seq_no] == false) {
			if (recv_len < 0) printf("ERROR WHILE RECEIVING !!");
			if (should_drop_this[rcv_pkt.seq_no] == false) {
				printf("RECEIVED PACKET %d :ACCEPT :BASE %d\n", rcv_pkt.seq_no, BASE);
				ack_array.push(rcv_pkt.seq_no);
				received_packets[rcv_pkt.seq_no] = true;
				ALL_DATA[rcv_pkt.seq_no] = rcv_pkt;
				packetQuere.push(rcv_pkt);
			} else {
				should_drop_this[rcv_pkt.seq_no] = false;
				printf("RECEIVED PACKET %d :DROP :BASE %d\n", rcv_pkt.seq_no, BASE);
				received_packets[rcv_pkt.seq_no] = false;
			}
		}

		for (int i = 0; i < NUMBER_OF_PACKETS; i++)
			if (received_packets[i] == false)
				all_received = false;
	}
}

void* sending(void* one) {
	while (!all_sent_ack) {
		all_sent_ack = true;
		while (ack_array.empty() == false) {
			ack_pkt.seq_no = ack_array.front();
			ack_array.pop();
			sent_ack[ack_pkt.seq_no] = true;
			if (sendto(sock, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &from, slen) < 0) printf("ERROR WHILE SENDING ACK !!");
			printf("SEND ACK %d\n", ack_pkt.seq_no);
		}
		for (int i = 0; i < NUMBER_OF_PACKETS; i++)
			if (sent_ack[i] == false)
				all_sent_ack = false;
	}
}

// send ./server.out 1 for drop
int main(int argc, char *argv[]) {

	if (argc != 2) printf("ENTER COMMAND-LINE ARGUMENTS !!");
	string s = argv[1];
	without_drop = (s == "0")? true : false;

	slen = sizeof(server);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) printf("ERROR WHILE CREATING SOCKET !!");
	printf("SOCKET CREATED !!");

	memset((char *) &server, 0, slen);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) printf("ERROR WHILE BINDING !!");
	printf("BINDING SUCCESSFULLY !!");

	char x[1];
	recv_len = recvfrom(sock, &x, sizeof(int), 0, (struct sockaddr *) &from, &slen);
	if (recv_len < 0) printf("ERROR WHILE RECEIVING WINDOW SIZE !!");
	WINDOW_SIZE = (int)x[0];
	printf("received WINDOW_SIZE = %d\n", WINDOW_SIZE);

	char nop[BUFFER_SIZE];
	recv_len = recvfrom(sock, &nop, BUFFER_SIZE, 0, (struct sockaddr *) &from, &slen);
	if (recv_len < 0) printf("ERROR WHILE RECEIVING NUMBER OF PACKETS !!");
	string tempp = nop;
	NUMBER_OF_PACKETS = stoi(tempp);
	printf("NUMBER OF PACKETS %d\n", NUMBER_OF_PACKETS);

	if (without_drop == true) dont_drop();

	pthread_t thread_one, thread_two, thread_three;

	pthread_create(&thread_one, NULL, &sending, (void *)one);
	pthread_create(&thread_two, NULL, &receiving, (void *)one);

	pthread_join(thread_one, NULL);
	pthread_join(thread_two, NULL);

	FILE *file_write = fopen("out.txt", "w");
	if (file_write == NULL) printf("ERROR WHILE OPENING FILE !!");

	int bsize = ALL_DATA.size();
	 for(int i = 0; i < bsize; i++){
		char *char_data = ALL_DATA[i].data;
		int L = strlen(char_data);
		for(int j = 0; i < L; i++) {
			fputc(char_data[j], file_write);
		}
	}
	printf("CHECK OUTPUT FILE NOW !!\n\n");
	
	return 0;
}
