#include <asm/byteorder.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>

int main(int argc, char **argv) {
	// socket structure
	struct sockaddr_in socket_struct;
	int struct_size_socket = sizeof socket_struct;
	socket_struct.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	socket_struct.sin_port = htons(8008);

	// creating server socket
	int socket_server = socket(socket_struct.sin_family = AF_INET, SOCK_STREAM,
	IPPROTO_TCP);
	// binding
	bind(socket_server, (void*) &socket_struct, struct_size_socket);

	// listen 10 connections
	listen(socket_server, 10);

	int socket_client;
	for (;; close(socket_client)) {
		socket_client = accept(socket_server, 0, 0);

		if (fork())
			continue;

		if (socket_client > 0) {
			char buffer_read[8192];
			char session_string[8192];
			char* html = "<!DOCTYPE html>\n<html></html>";
			size_t html_length = strlen(html);

			for (;;) {
				*session_string = 0;

				int response_status = 200;
				for (int request_line = 0;; request_line++) {
					int i = 0;
					int parameter = 1;
					while (i < 8192) {
						if (read(socket_client, buffer_read + i, 1) <= 0)
							return 0;
						if (buffer_read[i] == '\n')
							break;
						if (request_line && parameter)
							if (buffer_read[i] == ':')
								parameter = 0;
							else
								buffer_read[i] = toupper(buffer_read[i]);
						i++;
					}

					if (i < 8192) {
						if (buffer_read[i - 1] != '\r')
							i++;

						if (!request_line && *buffer_read != 'G')
							response_status = 501;
					} else
						response_status = 400;

					buffer_read[i - 1] = 0;

					size_t string_length = strlen(buffer_read);

					if (response_status != 200 || !string_length)
						break;

					char* substring = "COOKIE:";
					if (!strncmp(buffer_read, substring, strlen(substring))) {
						char* substring_with;
						substring = "session=";
						if (substring_with = strstr(buffer_read, substring))
							sscanf(substring_with + strlen(substring),
									"%s[^;\r\n]", session_string);
					}
				}

				long session_long = 0;
				int wait_length = 0;
				if (strlen(session_string)) {
					session_long = atol(session_string);

					if (session_long >= 0) {
						long char_number = session_long / 8;
						int bit_number = session_long % 8;

						if (html_length < char_number)
							session_long = -1;
						else {
							session_long++;
							wait_length = html[char_number] & 1 << bit_number;
						}
					}
				}

				if (wait_length)
					sleep(1);

				char response[8192];
				char content[8192];
				if (response_status == 200)
					snprintf(content, 8192,
							"Content-type: text/html\r\nSet-Cookie: session=%ld\r\n",
							session_long);

				long response_length = snprintf(response, 8192,
						"HTTP/1.0 %s\r\n%sContent-Length: %ld\r\n\r\n",
						response_status == 200 ? "200 OK" :
						response_status == 400 ?
								"400 Bad Request" : "501 Unsupported",
						response_status == 200 ? content : "",
						response_status == 200 ? html_length : 0);

				write(socket_client, response, response_length);

				if (response_status == 200)
					write(socket_client, html, html_length);
			}
		}
	}

	return 0;
}
