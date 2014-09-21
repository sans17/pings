#include <asm/byteorder.h>
#include <ctype.h>
#include <cygwin/in.h>
#include <cygwin/socket.h>
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
			printf("-4: request_string={%s}\n", request_string);

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

	printf("-3: session_string={%s}\n", session_string);
	if (strlen(session_string))
		session_int = atoi(session_string);
}

int bit_number = 0;

int main(int argc, char **argv) {
	int ret = 0;

	printf("-2: argc=%d\n", argc);

	struct sockaddr_in socket_struct;
	socket_struct.sin_family = AF_INET;
	int socket_main;

	char request[8192]; // read buffer
	char response[8192];
	struct timeval time_value;

	if (argc <= 1) {
		// read stdin
		int input[8192];
		int input_length = 0;
		for (; input_length < 8192; input_length++) {
			int input_int = fgetc(stdin);
			if (input_int == EOF)
				break;
			input[input_length] = input_int;
		}
		printf("-1: input_length=%d\n", input_length);

		long sessions[1000]; // bits sent
		for (int i = 0; i < 1000; i++)
			sessions[i] = -1;

		fd_set sockets_ready; // sockets ready to read
		FD_ZERO(&sockets_ready);
		FD_ZERO(&sockets_available);

		// socket structure
		socket_struct.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY is 0 anyway
		socket_struct.sin_port = htons(8008);

		// binding
		bind(socket_main = socket(AF_INET, SOCK_STREAM, 0), &socket_struct,
				sizeof socket_struct); // 0 works for protocol
		printf("0: socket_main=%d\n", socket_main);

		// listen 10 connections
		listen(socket_main, 10);

		FD_SET(socket_max = socket_main, &sockets_available);
		for (;;) {
			time_value.tv_sec = 5;
			time_value.tv_usec = 0;

			sockets_ready = sockets_available; // copy available to read from
			int selected_num = select(socket_max + 1, &sockets_ready, 0, 0,
					&time_value);  // 5 seconds timeout
			printf("0.3: selected_num=%d\n", selected_num);
			if (selected_num < 0)
				goto the_end;

			for (socket_num = 0; socket_num <= socket_max; socket_num++)
				if (FD_ISSET(socket_num, &sockets_available)) {
					printf("0.4: socket_num=%d\n", socket_num);
					if (FD_ISSET(socket_num, &sockets_ready))
						if (socket_num == socket_main) { // server - accepting new connection
							int socket_client = accept(socket_main, 0, 0); // not reporting anything back 0-s are ok
							FD_SET(socket_client, &sockets_available);
							if (socket_client > socket_max)
								socket_max = socket_client;
							printf("0.5: socket_client=%d, socket_max=%d\n",
									socket_client, socket_max);
							break;
						} else {
							printf("0.6: socket_num=%d\n", socket_num);

							int read_bytes = read(socket_num, request, 8192);
							if (read_bytes <= 0)
								close_socket();
							else {
								request[8191] = 0;
								if (*request == 'G') {
									int wait_length = 0;

									parse_session(request, "COOKIE:");

									if (session_int) {
										long char_number = sessions[session_int
												- 1] / 8;

										if (char_number > input_length) {
											sessions[session_int - 1] = 0;
											session_int = 0;
										} else {
											sessions[session_int - 1]++;
											wait_length =
													input[char_number]
															& 1
																	<< (bit_number =
																			sessions[session_int
																					- 1]
																					% 8);
										}
									} else
										for (int new_session_int = 0;
												new_session_int < 1000;
												new_session_int++)
											if (sessions[new_session_int]
													== -1) {
												sessions[new_session_int] = 0;
												session_int = new_session_int
														+ 1;
												break;
											}

									printf(
											"1: session_int=%d, wait_length=%d\n",
											session_int, wait_length);

									if (wait_length) {
										time_value.tv_sec = 0;
										time_value.tv_usec = 250000;

										select(0, 0, 0, 0, &time_value);
									}

									char* html =
											"<!DOCTYPE html>\n<html><body>A</body></html>";
									long html_length = strlen(html);
									long response_length =
											snprintf(response, 8192,
													"HTTP/1.0 200 OK\r\nContent-type: text/html\r\nSet-Cookie: session=%d\r\nContent-Length: %ld\r\n\r\n",
													session_int, strlen(html));
									printf("2: response={%s}\n", response);
									printf("3: html={%s}\n", html);
									write(socket_num, response,
											response_length);
									write(socket_num, html, html_length);
								}
							}
						}
					else if (socket_num != socket_main)
						close_socket();
				}
		}
	} else { // client
		printf("4: argv[1]={%s}\n", argv[1]);

		char address[8192];
		int address_length_1;
		sscanf(argv[1], "http://%8191[^/]%n", address, &address_length_1);

		int request_length = snprintf(request, 8192,
				"GET %s HTTP/1.0\r\nHost: %s\r\n",
				*(argv[1] + address_length_1) ?
						argv[1] + address_length_1 : "/", address);
		printf("4.5: request={%s}\n", request);

		printf("5: argv[1]={%s}, address={%s}, address_length_1=%d\n", argv[1],
				address, address_length_1);

		int address_length;
		int port = 80;
		sscanf(address, "%*[^:]%n:%d", &address_length, &port);
		address[address_length] = 0;
		printf("6: address={%s}, address_length=%d, port=%d\n", address,
				address_length, port);

		struct hostent *host = gethostbyname(address);
		memcpy(&socket_struct.sin_addr.s_addr, host->h_addr, host->h_length);
		socket_struct.sin_port = htons(port);

		int status = connect(socket_main = socket(AF_INET, SOCK_STREAM, 0),
				&socket_struct, sizeof socket_struct);
		printf("6: status=%d\n", status);
		if (!status)
			for (int read_int = 0;;) {
				int old_session_int = session_int;
				printf("6: old_session_int=%d\n", old_session_int);

				gettimeofday(&time_value, 0);

				write(socket_main, request, request_length);
				if (session_int) {
					char content[8192];
					int content_length = snprintf(content, 8192,
							"Cookie: session=%d\r\n", session_int);
					write(socket_main, content, content_length);
				}
				write(socket_main, "\r\n", 2);

				read(socket_main, response, 8192);

				struct timeval time_end;
				gettimeofday(&time_end, 0);

				long diff = (time_end.tv_sec - time_value.tv_sec) * 1000000
						+ time_end.tv_usec - time_value.tv_usec;

				response[8191] = 0;
				printf("6: response={%s}, diff=%ld\n", response, diff);

				parse_session(response, "SET-COOKIE:");
				if (old_session_int)
					if (session_int) {
						read_int |= (diff < 250000 ? 0 : 1)
								<< (bit_number = (bit_number + 1) % 8);
						if (!bit_number)
						{
							putchar(read_int);
							read_int = 0;
						}
					} else
						break;
				else if (!session_int)
					break;
			}
	};

	the_end: close(socket_main);
	return ret;
}
