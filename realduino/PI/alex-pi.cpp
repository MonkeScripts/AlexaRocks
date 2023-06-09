#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include "packet.h"
#include "serial.h"
#include "serialize.h"
#include "constants.h"
#include <ncurses.h>

#define PORT_NAME			"/dev/ttyACM0"
#define BAUD_RATE			B9600

int ncurse_flag = 1;
int exitFlag= 0;

sem_t _xmitSema;

void handleColour(TPacket *packet)
{
        printf("RGB: %d  %d  %d\n", packet->params[10], packet->params[11], packet->params[12]);
	printf("Distance: %d\n", packet->params[13]);
	printf("Hue: %d\n", packet->params[14]);
}

void handleError(TResult error)
{
	switch(error)
	{
		case PACKET_BAD:
			printf("ERROR: Bad Magic Number\n");
			break;

		case PACKET_CHECKSUM_BAD:
			printf("ERROR: Bad checksum\n");
			break;

		default:
			printf("ERROR: UNKNOWN ERROR\n");
	}
}

void handleResponse(TPacket *packet)
{
	// The response code is stored in command
	switch(packet->command)
	{
		case RESP_OK:
			printf("Command OK\n");
		break;

		case RESP_COLOUR:
			handleColour(packet);
		break;

		default:
			printf("Arduino is confused\n");
	}
}

void handleErrorResponse(TPacket *packet)
{
	// The error code is returned in command
	switch(packet->command)
	{
		case RESP_BAD_PACKET:
			printf("Arduino received bad magic number\n");
		break;

		case RESP_BAD_CHECKSUM:
			printf("Arduino received bad checksum\n");
		break;

		case RESP_BAD_COMMAND:
			printf("Arduino received bad command\n");
		break;

		case RESP_BAD_RESPONSE:
			printf("Arduino received unexpected response\n");
		break;

		default:
			printf("Arduino reports a weird error\n");
	}
}

void handleMessage(TPacket *packet)
{
	printf("Message from Alex: %s\n", packet->data);
}

void handlePacket(TPacket *packet)
{
	switch(packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
			// Only we send command packets, so ignore
			break;

		case PACKET_TYPE_RESPONSE:
				handleResponse(packet);
			break;

		case PACKET_TYPE_ERROR:
				handleErrorResponse(packet);
			break;

		case PACKET_TYPE_MESSAGE:
				handleMessage(packet);
			break;
	}
}

void sendPacket(TPacket *packet)
{
	char buffer[PACKET_SIZE];
	int len = serialize(buffer, packet, sizeof(TPacket));

	serialWrite(buffer, len);
}

void *receiveThread(void *p)
{
	char buffer[PACKET_SIZE];
	int len;
	TPacket packet;
	TResult result;
	int counter=0;

	while(1)
	{
		len = serialRead(buffer);
		counter+=len;
		if(len > 0)
		{
			result = deserialize(buffer, len, &packet);

			if(result == PACKET_OK)
			{
				counter=0;
				handlePacket(&packet);
			}
			else 
				if(result != PACKET_INCOMPLETE)
				{
					printf("PACKET ERROR\n");
					handleError(result);
				}
		}
	}
}

void flushInput()
{
	char c;
	while((c = getchar()) != '\n' && c != EOF);
}

void sendCommand(char command)
{
	TPacket commandPacket;
	commandPacket.packetType = PACKET_TYPE_COMMAND;

	switch(command)
	{     
		case 'e':
		case 'E':
			commandPacket.command = COMMAND_STOP;
			sendPacket(&commandPacket);
			break;

		case 'x':
		case 'X':
			commandPacket.command = COMMAND_DETECT_COLOUR;
			sendPacket(&commandPacket);
			break;
		
		case 'w':
		case 'W':
			commandPacket.command = COMMAND_F;
			sendPacket(&commandPacket);
			break;

		case 's':
                case 'S':
                        commandPacket.command = COMMAND_B;
                        sendPacket(&commandPacket);
                        break;

		case 'a':
                case 'A':
                        commandPacket.command = COMMAND_L;
                        sendPacket(&commandPacket);
                        break;

		case 'd':
                case 'D':
                        commandPacket.command = COMMAND_R;
                        sendPacket(&commandPacket);
                        break;

		case 'j':
		case 'J':
			commandPacket.command = COMMAND_DECREASE;
			sendPacket(&commandPacket);
			break;
		
		case 'h':
		case 'H':
			commandPacket.command = COMMAND_INCREASE;
			sendPacket(&commandPacket);
			break;

		case 'q':
		case 'Q':
			exitFlag=1;
			break;

		default:
			printf("Bad command\n");

	}
}

void printMessage(int speed) {
	printw("w, a, s, d to move.\ne to stop.\nh to increase speed.\nj to decrease speed.\npress o then enter to leave.\nSpeed is %d\n",speed);
}

int main()
{       // Connect to the Arduino
	startSerial(PORT_NAME, BAUD_RATE, 8, 'N', 1, 5);

	// Sleep for two seconds
	printf("WAITING TWO SECONDS FOR ARDUINO TO REBOOT\n");
	sleep(2);
	printf("DONE\n");

	// Spawn receiver thread
	pthread_t recv;

	pthread_create(&recv, NULL, receiveThread, NULL);

	// Send a hello packet
	TPacket helloPacket;

	helloPacket.packetType = PACKET_TYPE_HELLO;
	sendPacket(&helloPacket);

	int speed = 60;

	while(!exitFlag)
	{       
		char input;
		if (ncurse_flag == 1) {
			clear();
			initscr();
			cbreak();
			noecho();
			printMessage(speed);
		}

		while (ncurse_flag)
		{       
			input = getch();
			if (input == 'o') {
				ncurse_flag = 0;
				endwin();
				exitFlag = 0;
			} else if (input == 'w' || input == 'a' || input == 's' || input == 'd' || input == 'e' || input == 'h' || input == 'j' || input == 'x') {      
				if (input == 'h' & speed < 100) {
					speed += 10;
				} else if (input == 'j' & speed > 0) {
					speed -= 10;
				}
			        clear();
				printMessage(speed);
				refresh();
				sendCommand(input);
			}
		}

		char ch;
		printf("Command (e=stop, x=colour, q=exit, o=return to ncurses)\n");
		scanf("%c", &ch);

		// Purge extraneous characters from input stream
		flushInput();

		if (ch == 'o') {
			ncurse_flag = 1;
			refresh();
		} else {
			sendCommand(ch);
		}
	}
		printf("Closing connection to Arduino.\n");
		endSerial();
		endwin();
}
