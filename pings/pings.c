/**??/
/
//#include <asm/byteorder.h>
//#include <ctype.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/select.h>
//#include <sys/socket.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/unistd.h>

fd_set fd_available;
int fd_max;

void close_fd(int fd_num) {
	close(fd_num);
	FD_CLR(fd_num, &fd_available);
	if (fd_num >= fd_max)
		while (--fd_max)
			if (FD_ISSET(fd_max, &fd_available))
				break;
}

char session_string[8192];
char old_session_string[8192];

void parse_session(char* communication, char* substring_cookie) {
	session_string[0] = 0;

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
}

int pause_length = 500000;
int pause_bumper = 1000;

int main(int argc, char **argv) {
	int ret = 0;
	session_string[0] = 0;

	struct sockaddr_in socket_struct;
	socket_struct.sin_family = AF_INET; // AF_INET = 2
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
		memcpy(&socket_struct.sin_addr.s_addr, host->h_addr_list[0],
				host->h_length);
		socket_struct.sin_port = htons(port);

		for (int read_int = 0, bit_number = 0;; close(socket_main))
			if (!connect(socket_main = socket(AF_INET, SOCK_STREAM, 0),
					&socket_struct, sizeof socket_struct)) // AF_INET=2, SOCK_STREAM=1, 0 works for protocol
				for (;;) {
					strcpy(old_session_string, session_string);

					char content[8192];
					int content_length = snprintf(content, 8192,
							"%sCookie: session=%s\r\n\r\n", request,
							session_string);

					if (write(socket_main, content, content_length) < 0)
						break;

					gettimeofday(&time_value, 0);

					response[0] = 0;
					int read_length = read(socket_main, response, 8192);
					if (read_length < 0)
						break;
					else if (read_length && *response == 'H') {
						struct timeval time_end;

						gettimeofday(&time_end, 0);

						response[8191] = 0;

						parse_session(response, "SET-COOKIE:");

						if (old_session_string[0])
							if (!strncmp(old_session_string, session_string,
									strlen(old_session_string))) {

								int increment = 0;
								// some buffer
								if ((time_end.tv_sec - time_value.tv_sec)
										* 1000000 + time_end.tv_usec
										- time_value.tv_usec < pause_length - pause_bumper) {
									increment = bit_number < 15 ? 1 : 16;
									bit_number += increment;
									read_int += increment;
								} else
									bit_number = bit_number < 15 ? 15 : 255;

								if (bit_number == 255) {
									putchar(read_int);
									fflush(stdout);
									read_int = 0;
									bit_number = 0;
								}
							} else {
								ret = 1;
								goto the_end;
							}

						// test of client breaking connection
//						break;
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

		int session_info[8192][2]; // info sent
		int pipes[8192][2];
		for (int i = 0; i < 8192; i++) {
			session_info[i][0] = session_info[i][1] = -1;
			pipes[i][0] = pipes[i][1] = 0;
		}

		gettimeofday(&time_value, 0);
		char secs_string[8192];
		int secs_string_length = sprintf(secs_string, "%ld", time_value.tv_sec);

		time_value.tv_sec = 0;
		time_value.tv_usec = pause_length;

		fd_set fd_ready; // sockets ready to read
		fd_set fd_sockets;
		FD_ZERO(&fd_ready);
		FD_ZERO(&fd_available);
		FD_ZERO(&fd_sockets);

		// socket structure
		socket_struct.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY=0
		socket_struct.sin_port = htons(8008);

		// binding
		bind(socket_main = socket(AF_INET, SOCK_STREAM, 0), &socket_struct,
				sizeof socket_struct); // AF_INET=2, SOCK_STREAM=1, 0 works for protocol

		// listen 10 connections
		listen(socket_main, 10);

		FD_SET(fd_max = socket_main, &fd_available);
		for (int parent_flag = 1, socket_client = 0;;) {
			fd_ready = fd_available; // copy available to read from
			int selected_num = select(fd_max + 1, &fd_ready, 0, 0, 0); // no timeout

			if (selected_num <= 0) {
				if (!parent_flag && socket_client)
					close_fd(socket_client);
				goto the_end;
			}

			for (int fd_num = socket_client; selected_num && fd_num <= fd_max;
					fd_num++)
				if (FD_ISSET(fd_num, &fd_ready)) {
					selected_num--;

					if (fd_num == socket_main) { // server - accepting new connection
						socket_client = accept(socket_main, 0, 0); // not reporting anything back 0-s are ok
						FD_SET(socket_client, &fd_available);
						FD_CLR(socket_client, &fd_ready);
						FD_SET(socket_client, &fd_sockets);
						if (socket_client > fd_max)
							fd_max = socket_client;

						socket_client = 0;
					} else if (!parent_flag || FD_ISSET(fd_num, &fd_sockets)) {
						int read_count = read(fd_num, request, 8192);

						int session_int = 0;
						if (read_count > 0) {
							request[8191] = 0;
							if (*request == 'G') {
								int wait_flag = 0;

								strcpy(old_session_string, session_string);

								parse_session(request, "COOKIE:");

								if (!strncmp(session_string, secs_string,
										strlen(secs_string))
										&& !strncmp(old_session_string,
												session_string,
												strlen(old_session_string))) {
									session_int = atoi(
											session_string
													+ secs_string_length);
								}

								if (session_int) {
									if (session_info[session_int][0]
											< input_length) {
										if (wait_flag =
												(session_info[session_int][1]
														< 15 ?
														session_info[session_int][1] :
														session_info[session_int][1]
																>> 4)
														>= (session_info[session_int][1]
																< 15 ?
																input[session_info[session_int][0]]
																		& 15 :
																input[session_info[session_int][0]]
																		>> 4))
											session_info[session_int][1] =
													session_info[session_int][1]
															< 15 ? 15 : 255;
										else
											session_info[session_int][1] +=
													session_info[session_int][1]
															< 16 ? 1 : 16;

										if (session_info[session_int][1]
												== 255) {
											session_info[session_int][0]++;
											session_info[session_int][1] = 0;
										}
									} else { // everything written
										write(pipes[session_int][1],
												session_info[session_int],
												sizeof(session_info[session_int])); // child process write what has been done
										close(pipes[session_int][1]);

										session_int = 0;
									}
								} else
									for (int new_session_int = 1;
											new_session_int < 8192;
											new_session_int++)
										if (session_info[new_session_int][0]
												== -1) {
											session_info[new_session_int][0] =
													session_info[new_session_int][1] =
															0;
											session_int = new_session_int;
											break;
										}

								char* html =
										"<div style='position:absolute;top:45%;left:40%;'>Obscurity is an illusion.</div>";
								if (session_int)
									sprintf(session_string, "%s%d", secs_string,
											session_int);
								else
									session_string[0] = 0;
								long response_length =
										snprintf(response, 8192,
												"HTTP/1.0 200 OK\r\nContent-type: text/html\r\nExpires: 0\r\nSet-Cookie: session=%s\r\nContent-Length: %ld\r\n\r\n%s",
												session_string, strlen(html),
												html);

								if (parent_flag) {
									pipe(pipes[session_int]);
									if (!(parent_flag = fork())) // child process
									{
										close(pipes[session_int][0]);

										FD_ZERO(&fd_available);
										FD_SET(socket_client = fd_max = fd_num,
												&fd_available);
									}
								}

								if (parent_flag < 0) {
									ret = 1;
									goto the_end;
								}
								if (parent_flag) { // parent process
									close(pipes[session_int][1]);
									FD_SET(pipes[session_int][0],
											&fd_available);
									if (pipes[session_int][0] > fd_max)
										fd_max = pipes[session_int][0];

									close_fd(fd_num);
									break;
								} else { // child process
									if (wait_flag) {
										time_value.tv_sec = 0;
										time_value.tv_usec = pause_length;

										select(0, 0, 0, 0, &time_value);
									}

									write(fd_num, response, response_length);

									if (!session_int) {
										close(fd_num);
										return 0;
									}
								}
							}
						} else if (parent_flag) {
							close(fd_num);
							FD_CLR(fd_num, &fd_sockets);

							close_fd(fd_num);
						} else {
							write(pipes[session_int][1],
									session_info[session_int],
									sizeof(session_info[session_int])); // child process write what has been done
							close(pipes[session_int][1]);
							close(fd_num);

							return 0;
						}
					} else { // not socket
						for (int session_int = 0; session_int <= fd_max;
								session_int++)
							if (pipes[session_int][0] == fd_num) {
								read(fd_num, session_info[session_int],
										sizeof(session_info[session_int]));

								close_fd(fd_num);

								if (session_info[session_int][0]
										>= input_length) {
									session_info[session_int][0] =
											session_info[session_int][1] = -1;
									pipes[session_int][0] =
											pipes[session_int][1] = 0;
								}
								break;
							}
					}
				}
		}
	};

	the_end: close(socket_main);
	return ret;
}
