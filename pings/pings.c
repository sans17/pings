#include <asm/byteorder.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>

int main(int argc, char **argv) {
	int ret = 0;
	if (argc <= 1) {
		// read stdin
		int input[8192];
		int input_length = 0;
		for (; input_length < 8192; input_length++) {
			int input_int = fgetc(stdin);
			if (input_int == EOF)
				break;
			input_int[input_length] = input_int;
		}

		long sessions[1000]; // bits sent
		for (int i = 0; i < 1000; i++)
			sessions[i] = 0;

		fd_set sockets_set; // sockets ready to read
		fd_set sockets_available;
		int socket_max;
		FD_ZERO(&sockets_set);
		FD_ZERO(&sockets_available);

		// socket structure
		struct sockaddr_in socket_struct;
		socket_struct.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY is 0 anyway
		socket_struct.sin_port = htons(8008);

		// creating server socket
		int socket_server = socket(socket_struct.sin_family = AF_INET,
		SOCK_STREAM, 0); // 0 works for protocol
		printf("0: socket_server=%d\n", socket_server);
		// binding
		bind(socket_server, &socket_struct, sizeof socket_struct);

		// listen 10 connections
		listen(socket_server, 10);

		FD_SET(socket_max = socket_server, &sockets_available);
		for (;;) {
			int selected_num = select(socket_max + 1, &(sockets_set =
					sockets_available), 0, 0, 0); // copy available to read from
			if (selected_num < 0)
				goto the_end;

			for (int socket_num = 0;
					selected_num > 0 && socket_num <= socket_max; socket_num++)
				if (FD_ISSET(socket_num, &sockets_set)) {
					socket_num--;
					if (socket_num == socket_server) { // server - accepting new connection
						int socket_client = accept(socket_server, 0, 0); // not reporting anything back 0-s are ok
						FD_SET(socket_client, &sockets_available);
						if (socket_client > socket_max)
							socket_max = socket_client;
					} else {
						char buffer_read[8192]; // read buffer

						int read_bytes = read(socket_num, buffer_read, 8192);
						if (read_bytes <= 0) {
							close(socket_num);
							FD_CLR(socket_num, &sockets_available);
							if (socket_num == socket_max)
								while (--socket_max)
									if (FD_ISSET(socket_max,
											&sockets_available))
										break;
						} else {
							buffer_read[8191] = 0;
							if (*buffer_read == 'G') {
								char session_string[8192];
								int session_int = 0;
								session_string[0] = 0;
								int wait_length = 0;

								char *read_pointer = strstr(buffer_read, "\n");
								if (read_pointer) {
									for (char *request_string_start =
											++read_pointer; *read_pointer;
											read_pointer++)
										if (*read_pointer == '\n') {
											*read_pointer = 0;
											if (*(read_pointer - 1) == '\r')
												*(read_pointer - 1) = 0;
											if (!strlen(request_string_start))
												break;

											char *substring_cookie = "COOKIE:";
											char *substring_session = "SESSION=";
											char *substring_with;
											if (!strncmp(request_string_start,
													substring_cookie,
													strlen(substring_cookie))
													&& (substring_with =
															strstr(
																	request_string_start,
																	substring_session))) {
												sscanf(
														substring_with
																+ strlen(
																		substring_session),
														"%s[^;\r\n]",
														session_string);
												break;
											}
											request_string_start = read_pointer
													+ 1;
										} else
											*read_pointer = toupper(
													*read_pointer);

									if (strlen(session_string))
										if (session_int = atoi(
												session_string)) {
											long char_number =
													sessions[session_int - 1]
															/ 8;
											int bit_number =
													sessions[session_int - 1]
															% 8;

											if (char_number > input_length) {
												sessions[session_int - 1] = 0;
												session_int = 0;
											} else {
												sessions[session_int - 1]++;
												wait_length = input[char_number]
														& 1 << bit_number;
											}
										}
								} else
									for (int new_session_int = 0;
											new_session_int < 1000;
											new_session_int++)
										if (!sessions[new_session_int]) {
											session_int = new_session_int + 1;
											break;
										}

								if (wait_length) {
									struct timeval wait_time;
									wait_time.tv_sec = 0;
									wait_time.tv_usec = 250000;

									select(0, 0, 0, 0, &wait_length);
								}

								char response[8192];
								char content[8192];
								char* html = "<!DOCTYPE html>\n<html></html>";
								long html_length = strlen(html);
								if (session_int)
									snprintf(content, 8192,
											"Content-type: text/html\r\nSet-Cookie: session=%d\r\n",
											session_int);

								long response_length =
										snprintf(response, 8192,
												"HTTP/1.0 200 OK\r\n%sContent-Length: %ld\r\n\r\n",
												session_int ? content : "",
												strlen(html));
								write(socket_num, response, response_length);
								write(socket_num, html, html_length);
							}
						}
					}
				}
		}

		the_end: close(socket_server);
	} else
		; // client not implemented yet

	return ret;
}
