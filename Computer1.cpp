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
#define THREE_SECOND 3.0

struct sockaddr_in client;
int sock;
int one = 1;
bool thread_running;


bool sent_packets[20000];
bool received_ack[20000];
bool acks_received;
bool all_received;



unsigned int WINDOW_SIZE = 5;
unsigned int NUMBER_OF_PACKETS;
unsigned int packet_number, BASE;
unsigned int expected;

map <int,vector<char>> HashMap;

socklen_t slen;


typedef struct packet1 {
	int seq_no;
} ACK_PKT;

typedef struct packet2 {
	int seq_no;
	char data[BUFFER_SIZE];
	int packet_size;
} DATA_PKT;

ACK_PKT rcv_ack;
DATA_PKT send_pkt[1000];
queue <packet2> packetQuere;


void* sending(void* one) {
	thread_running = true;
	BASE = 0;
	packet_number = 0;
	for(int i = 0; i < WINDOW_SIZE; i++){
		DATA_PKT current_packet = send_pkt[packet_number];
		packetQuere.push(current_packet);
		if (sendto(sock, &current_packet, sizeof(current_packet), 0, (struct sockaddr *) &client, slen) == -1) printf("ERROR WHILE SENDING PACKET !!");
		sent_packets[packet_number] = true;
		printf("SEND PACKET %d \t BASE %d\n", packet_number, BASE);
	 	packet_number++;
	}

	expected = 0;
	clock_t begin_time = clock();
	while (!acks_received) {
		acks_received = true;

		if (received_ack[expected] == true) {
			while (received_ack[expected] == true) {
				packetQuere.pop();
				BASE++;
				expected++;
			}

			while (packetQuere.size() < WINDOW_SIZE) {
				DATA_PKT current_packet = send_pkt[packet_number];
				packetQuere.push(current_packet);
				if (sendto(sock, &current_packet, sizeof(current_packet), 0, (struct sockaddr *) &client, slen) == -1) printf("ERROR WHILE SENDING PACKET !!");
				sent_packets[packet_number] = true;
				printf("SEND PACKET %d \t BASE %d\n", packet_number, BASE);
				packet_number++;
			}
			begin_time = clock();
		}

		float current_time = float( clock () - begin_time ) / CLOCKS_PER_SEC;
		if (current_time > THREE_SECOND) {
			begin_time = clock();
			DATA_PKT new_packet = send_pkt[expected];
			if (sendto(sock, &new_packet, sizeof(new_packet), 0, (struct sockaddr *) &client, slen) == -1) printf("ERROR WHILE SENDING PACKET !!");
			sent_packets[expected] = true;
			printf("TIMEOUT %d\n", expected);
			printf("RE-SEND PACKET %d \t BASE %d\n", expected, BASE);
		}
		for(int i = 0; i < NUMBER_OF_PACKETS; i++){
			if (received_ack[i] == false)
				acks_received = false;
		}
	}
	printf("ALL PACKETS ARE SENT !!\n");
}

void* receiving(void* one) {
	while (all_received == false) {
		all_received = true;
		if (recvfrom(sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &client, &slen) == -1) printf("ERROR WHILE RECEIVING DATA !!");	
		received_ack[rcv_ack.seq_no] = true;
		printf("RECEIVED ACK %d\n", rcv_ack.seq_no);
		for(int i = 0; i < NUMBER_OF_PACKETS; i++){
			if (received_ack[i] == false)
				all_received = false;
		}
	}
	printf("ALL ACKS ARE RECEIVED !!\n");
}

int main() {
	slen = sizeof(client);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) printf("** ERROR WHILE CREATING SOCKET !!");
	printf("SOCKET CREATED !!");

	memset((char *) &client, 0, slen);

	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr.s_addr = inet_addr("127.0.0.1");

	FILE *file = fopen("in.txt", "r");
	unsigned long long int total_characters = 0;

	if (file == NULL) {
		printf("* ERROR WHILE OPENING FILE\n");
		exit(0);
	}

	vector <char> v;
	int c;
	int sz = 0;
	int Q = 0;

	while ((c = fgetc(file)) != EOF) {
		total_characters++;
		sz++;
		v.push_back((char)c);
		if (sz == BUFFER_SIZE-4) {
			HashMap[Q++] = v;
			sz = 0;
			v.clear();
		}
	}

	if (sz > 0) HashMap[Q++] = v;

	NUMBER_OF_PACKETS = Q;


	printf("--- TOTAL NUMBER OF PACKETS %d\n", NUMBER_OF_PACKETS);

	for (int i = 0; i < Q; i++) {
		send_pkt[i].seq_no = i;
		vector < char > temp = HashMap[i];
		for(int k = 0; k < temp.size(); k++) send_pkt[i].data[k] = temp[k];
		send_pkt[i].packet_size = temp.size();
	}

	char wsize[1];
	wsize[0] = (char)WINDOW_SIZE;
	if (sendto(sock, &wsize, sizeof(WINDOW_SIZE), 0, (struct sockaddr *) &client, slen) == -1) printf("** ERROR WHILE SENDING WINDOW SIZE !!");
	printf("Window size = %d sent to server !!\n", WINDOW_SIZE);

	string str_int = to_string(NUMBER_OF_PACKETS);
	int NN = str_int.length();
	char nop[NN];
	strcpy(nop, str_int.c_str());
	if (sendto(sock, &nop, NN, 0, (struct sockaddr *) &client, slen) == -1) printf("** ERROR WHILE SENDING NUMBER OF PACKETS !!");
	printf("TOTAL NUMBER OF PACKETS ARE %d\n", NUMBER_OF_PACKETS);

 // two thread for recive and ack

	pthread_t thread_one, thread_two;

	pthread_create(&thread_one, NULL, &sending, (void *)one);
	pthread_create(&thread_two, NULL, &receiving, (void *)one);

	pthread_join(thread_one, NULL);
	pthread_join(thread_two, NULL);

	return 0;
}

