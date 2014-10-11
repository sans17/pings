#include <netdb.h>
#include <stdio.h>
#include <sys/time.h>

int M = 500000, N = 1000, x[O], o[O][2], m[O][2], Q, X, Y, Z, b, h, e, d, g, D;
fd_set A, t, u;
char E[O], F[O], T[O], U[O], W[O], c[O], *i = "", *H = "session=";
struct sockaddr_in R;
struct timeval V, f;
struct hostent *a;
long q;

void L(char* F) {
  0[E] = 0;
  char *J;
	if(J = strstr(F, H)) sscanf(J + 8, "%s[^;\r\n]", E);
}

int main(int S, char **P) {

	R.sin_family = 2;

	if (S > 1) {
		sscanf(1[P], "http://%8191[^/]%n", W, &X);
		snprintf(T, O, "GET %s HTTP/1.0\r\nHost: %s\r\n",
				*(1[P] + X) ? 1[P] + X : "/", W);
		X[W] = 0;
		a = gethostbyname(W);
		memcpy(&R.sin_addr.s_addr, 0[a->h_addr_list], a->h_length);
		R.sin_port = htons(80);
		for (;;k S))
			if (!connect j)
				while(1) {
					strcpy(F, E);
					d = snprintf(c, O, "%sCookie: %s%s\r\n\r\n", T, H, E);
					if (write(S, c, d) < 0)
						break;
					gettimeofday(&V, 0);
					0[U] = 0;
					e = read(S, U, O);
					if (e < 0)
						break;
					else if (e && *U == 72) {
						gettimeofday(&f, 0);
						U[O-1] = 0;
						L(U);
						if (0[F])
							if (!strncmp(F, E, strlen(F))) {
								g = 0;
								if ((f.tv_sec - V.tv_sec) * 1000000 + f.tv_usec
										- V.tv_usec < M - N) {
									h += g = h < 15 ? 1 : 16;
									b += g;
								} else
									h = h < 15 ? 15 : 255;
								if (h == 255) {
									putchar(b);
									fflush(stdout);
									b = h = 0;
								}
							} else {
								Q = 1;
								goto l;
							}
					}
				}
	} else {
		Z = 0;
		while(Z < O) {
			if ((Y = fgetc(stdin)) == -1)
				break;
			x[Z++] = Y;
		}
		Y = 0;
		while(Y < O) {
			0[Y[o]] = 1[Y[o]] = -1;
			0[Y[m]] = 1[Y[m]] = 0;
			Y++;
		}
		gettimeofday(&V, 0);		
		h = sprintf(c, "%ld", V.tv_sec);
		FD_ZERO(&t);
		FD_ZERO(&A);
		FD_ZERO(&u);
		R.sin_addr.s_addr = 0;
		R.sin_port = htons(80);
		bind j;
		listen(S, 10);
		FD_SET(S, &A);
		Y = 1;
		g = 0;
		while(1) {
			t = A;
			if (select(64, &t, 0, 0, 0) <= 0) {
				if (!Y && g)
				{
					k g);
					C g, &A);
				}
				goto l;
			}
			for (D = 0; D < 64; D++)
				if (FD_ISSET(D, &t))
					if (D == S) {
						g = accept(S, 0, 0);
						FD_SET(g, &A);
						C g, &t);
						FD_SET(g, &u);
						g = 0;
					} else if (!Y || FD_ISSET(D, &u)) {
						e = read(D, T, O);

						if (e > 0) {
							T[O-1] = 0;
							if (*T == 71) {
								int _ = 0;
								strcpy(F, E);
								L(T);
								if (X = strncmp(E, c, strlen(c)) || strncmp(F, E, strlen(F)) ? 0 : atoi(E + h)) {
									if (0[X[o]]	< Z) {
										if (_ = (1[X[o]] < 15 ? 1[X[o]] : 1[X[o]] >> 4)
														>= (1[X[o]]	< 15 ? 0[X[o]][x] & 15 : 0[X[o]][x]>> 4))
											1[X[o]] = 1[X[o]] < 15 ? 15 : 255;
										else
											1[X[o]] += 1[X[o]] < 16 ? 1 : 16;
										if (o[X][1] == 255) {
											0[X[o]]++;
											1[X[o]] = 0;
										}
									} else {
										write(1[X[m]] y
              			k 1[X[m]]);
										X = 0;
									}
								} else
									for (d = 1; d < O; d++)
										if (0[d[o]]	< 0) {
											0[d[o]] =	1[o[X = d]] =	0;
											break;
										}
								0[E] = 0;
								if (X)
									sprintf(E, "%s%d", c, X);
								q = snprintf(U, O,
												"HTTP/1.0 200 OK\r\nContent-type: text/html\r\nExpires: 0\r\nSet-Cookie: %s%s\r\nContent-Length: %ld\r\n\r\n%s",
												H, E, strlen(i),
												i);
								if (Y) {
									pipe(m[X]);
									if (!(Y = fork())) {
										k 0[X[m]]);
										FD_ZERO(&A);
										FD_SET(g = D,	&A);
									}
								}
								if (Y < 0) {
									Q = 1;
									goto l;
								}
								if (Y) {
									k 1[X[m]]);
									FD_SET(0[X[m]], &A);
									k D);
									C D, &A);
									break;
								} else {
									if (_) {
										V.tv_sec = 0;
										V.tv_usec = M;
										select(0, 0, 0, 0, &V);
									}
									write(D, U, q);
									if (!X) {
										k D);
										return 0;
									}
								}
							}
						} else if (Y) {
							C D, &u);
							k D);
							C D, &A);
						} else {
							write(1[X[m]] y
							k 1[X[m]]);
							k D);
							return 0;
						}
					} else
						for (X = 0; X < O; X++)
							if (0[X[m]] == D) {
								read(D y
								k D);
								C D, &A);
								if (o[X][0]	>= Z) {
									0[X[o]] = 1[X[o]] = -1;
									0[X[m]] =	1[X[m]] = 0;
								}
								break;
							}
		}
	}

	l: k S);
	return Q;
}
