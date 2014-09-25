#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/unistd.h>

int socket_num;
int socket_max;
fd_set sockets_available;

void close_socket() {
	close(socket_num);
	FD_CLR(socket_num, &sockets_available);
	if (socket_num == socket_max)
		while (--socket_max)
			if (FD_ISSET(socket_max, &sockets_available))
				break;
}

char session_string[8192];
int session_int = 0;

void parse_session(char* communication, char* substring_cookie) {
	session_string[0] = 0;
	session_int = 0;

	for (char *read_pointer = communication, *request_string = communication;
			*read_pointer; read_pointer++) {
		if (*read_pointer == '\n') {
			if (read_pointer - request_string
					== (*(read_pointer - 1) == '\r' ? 2 : 1))
				break;
			*read_pointer = 0;

			char *substring_session = "SESSION=";
			char *substring_with;
			if (!strncmp(request_string, substring_cookie,
					strlen(substring_cookie))
					&& (substring_with = strstr(request_string,
							substring_session))) {
				sscanf(substring_with + strlen(substring_session), "%s[^;\r\n]",
						session_string);
				break;
			}
			request_string = read_pointer + 1;
		} else
			*read_pointer = toupper(*read_pointer);
	}

	if (strlen(session_string))
		session_int = atoi(session_string);
}

int main(int argc, char **argv) {
	int ret = 0;

	struct sockaddr_in socket_struct;
	socket_struct.sin_family = AF_INET;
	int socket_main;

	char request[8192]; // read buffer
	char response[8192];
	struct timeval time_value;

	if (argc > 1) { // client
		char address[8192];
		int address_length_1;
		sscanf(argv[1], "http://%8191[^/]%n", address, &address_length_1);

		snprintf(request, 8192, "GET %s HTTP/1.0\r\nHost: %s\r\n",
				*(argv[1] + address_length_1) ?
						argv[1] + address_length_1 : "/", address);

		int address_length;
		int port = 80;
		sscanf(address, "%*[^:]%n:%d", &address_length, &port);
		address[address_length] = 0;

		struct hostent *host = gethostbyname(address);
		memcpy(&socket_struct.sin_addr.s_addr, host->h_addr, host->h_length);
		socket_struct.sin_port = htons(port);

		if (!connect(socket_main = socket(AF_INET, SOCK_STREAM, 0),
				&socket_struct, sizeof socket_struct))
			for (int read_int = 0, bit_number = 0;;) {
				int old_session_int = session_int;
				//				printf("6: old_session_int=%d\n", old_session_int);

				char content[8192];
				int content_length = snprintf(content, 8192,
						"%sCookie: session=%s\r\n\r\n", request,
						session_int ? session_string : "");
				write(socket_main, content, content_length);

				gettimeofday(&time_value, 0);

				read(socket_main, response, 8192);
				if (*response == 'H') {
					struct timeval time_end;
					gettimeofday(&time_end, 0);

					long diff = (time_end.tv_sec - time_value.tv_sec) * 1000000
							+ time_end.tv_usec - time_value.tv_usec;

					printf("diff=%ld, bit_number=%d, read_int=%d\n", diff,
							bit_number, read_int);

					response[8191] = 0;

					parse_session(response, "SET-COOKIE:");
					if (old_session_int)
						if (session_int) {
							if (diff < 250000) {
								bit_number++;
								if (bit_number == 16)
									read_int |= 15;
								else if (bit_number == 32)
									read_int |= 240;
							} else {
								read_int += (
										bit_number < 16 ?
												bit_number : bit_number << 4);
								bit_number = 16;
							}
							printf("bit_number=%d, read_int=%d\n", bit_number,
									read_int);

							if (bit_number == 32) {
								putchar(read_int);
								fflush(stdout);
								read_int = 0;
								bit_number = 0;
							}
						} else
							break;
					else if (!session_int)
						break;
				}
			}
	} else {
		// read stdin
		int input[8192];
		int input_length = 0;
		for (; input_length < 8192; input_length++) {
			int input_int = fgetc(stdin);
			if (input_int == EOF)
				break;
			input[input_length] = input_int;
		}

		long sessions[1000][2]; // info sent
		for (int i = 0; i < 1000; i++)
			sessions[i][0] = sessions[i][1] = -1;

		fd_set sockets_ready; // sockets ready to read
		FD_ZERO(&sockets_ready);
		FD_ZERO(&sockets_available);

		// socket structure
		socket_struct.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY is 0 anyway
		socket_struct.sin_port = htons(8008);

		// binding
		bind(socket_main = socket(AF_INET, SOCK_STREAM, 0), &socket_struct,
				sizeof socket_struct); // 0 works for protocol

		// listen 10 connections
		listen(socket_main, 10);

		FD_SET(socket_max = socket_main, &sockets_available);
		for (;;) {
			time_value.tv_sec = 5;
			time_value.tv_usec = 0;

			sockets_ready = sockets_available; // copy available to read from
			int selected_num = select(socket_max + 1, &sockets_ready, 0, 0,
					&time_value);  // 5 seconds timeout
			if (selected_num < 0)
				goto the_end;

			for (socket_num = 0; socket_num <= socket_max; socket_num++)
				if (FD_ISSET(socket_num, &sockets_available)) {
					if (FD_ISSET(socket_num, &sockets_ready))
						if (socket_num == socket_main) { // server - accepting new connection
							int socket_client = accept(socket_main, 0, 0); // not reporting anything back 0-s are ok
							FD_SET(socket_client, &sockets_available);
							if (socket_client > socket_max)
								socket_max = socket_client;
							break;
						} else {
							int read_bytes = read(socket_num, request, 8192);
							if (read_bytes <= 0)
								close_socket();
							else {
								request[8191] = 0;
								if (*request == 'G') {
									int wait_length = 0;

									parse_session(request, "COOKIE:");

									if (session_int) {
										if (sessions[session_int][0]
												>= input_length) {
											sessions[session_int][0] =
													sessions[session_int][1] =
															-1;
											session_int = 0;
										} else {
//											printf(
//													"0: input[sessions[session_int][0]]=%d, sessions[session_int][1]=%d\n",
//													input[sessions[session_int][0]],
//													sessions[session_int][1]);
											if (wait_length =
													(sessions[session_int][1]
															< 15 ?
															sessions[session_int][1] :
															sessions[session_int][1]
																	>> 4)
															>= (sessions[session_int][1]
																	< 15 ?
																	input[sessions[session_int][0]]
																			& 15 :
																	input[sessions[session_int][0]]
																			>> 4))
												sessions[session_int][1] =
														sessions[session_int][1]
																< 15 ? 15 : 255;
											else
												sessions[session_int][1] +=
														sessions[session_int][1]
																< 16 ? 1 : 16;
//											printf(
//													"1: sessions[session_int][1]=%d, wait_length=%d\n",
//													sessions[session_int][1],
//													wait_length);
											if (sessions[session_int][1]
													== 255) {
												sessions[session_int][0]++;
												sessions[session_int][1] = 0;
											}
										}
									} else
										for (int new_session_int = 1;
												new_session_int < 1000;
												new_session_int++)
											if (sessions[new_session_int][0]
													== -1) {
												sessions[new_session_int][0] =
														sessions[new_session_int][1] =
																0;
												session_int = new_session_int;
												break;
											}

									if (wait_length) {
										time_value.tv_sec = 0;
										time_value.tv_usec = 250000;

										select(0, 0, 0, 0, &time_value);
									}

									char* html =
											"<!DOCTYPE html>\n<html><body>A</body></html>";
									long response_length =
											snprintf(response, 8192,
													"HTTP/1.0 200 OK\r\nContent-type: text/html\r\nSet-Cookie: session=%d\r\nContent-Length: %ld\r\n\r\n%s",
													session_int, strlen(html),
													html);
									write(socket_num, response,
											response_length);
								}
							}
						}
					else if (socket_num != socket_main)
						close_socket();
				}
		}
	};

	the_end: close(socket_main);
	return ret;
}
