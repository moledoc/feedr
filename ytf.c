#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// TODO: logging
// TODO: better responses
// TODO: convenience func to connect to socket. For now I'll type out manually for practice.
// TODO: help function
// MAYBE: TODO: testing; could have a special endpoint that swaps to "local db" instance and can run deterministic tests against that.

int handle_health(char *msg) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/ytfd.health.sock", sizeof(addr.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: failed to connect to socket: %s\n", strerror(errno));
		return errno;
	}

	write(sockfd, msg, strlen(msg));

	char resp[256];
	ssize_t n = read(sockfd, resp, 256);
	printf("%s\n", resp);
	if (close(sockfd) == -1) {
		fprintf(stderr, "[WARNING]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_sub(char *chan_name) {
	int status = 0;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/ytfd.add.sock", sizeof(addr.sun_path)-1);
	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	int n = write(sockfd, chan_name, strlen(chan_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	char buf[256];
	n = read(sockfd, buf, 256);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: read sub: %s\n", strerror(errno));
		return errno;
	}
	if (n < 256) {
		buf[n] = '\0';
	}
	if (n != 0) {
		status = ENOMEDIUM;
	}	

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: failed to close socket: %s\n", strerror(errno));
		return errno;
	}
	return status;		
}

int handle_search(char *channel_name) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: socket search: %s\n", strerror(errno));
		return errno;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/ytfd.search.sock", sizeof(addr.sun_path)-1);
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: connect search: %s\n", strerror(errno));
		return errno;
	}

	write(sockfd, channel_name, strlen(channel_name));
	char buf[256];
	int n = read(sockfd, buf, 256);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: read search: %s\n", strerror(errno));
		return errno;
	}
	if (n < 256) {
		buf[n] = '\0';
	}
	fprintf(stdout, "%s\n", buf);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: close search: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_unsub(char *channel_name) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/ytfd.rm.sock", sizeof(addr.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	write(sockfd, channel_name, strlen(channel_name));

	// TODO: read response. needs response from daemon though.

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_subs() {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, "/tmp/ytfd.subs.sock", sizeof(addr.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	char buf[256];
	int n = read(sockfd, buf, 256);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: failed to read subs: %s\n", strerror(errno));
		return errno;
	}

	if (n < 256) {
		buf[n] = '\0';
	}
	printf("%s", buf);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_fetch(char *channel_name) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	struct sockaddr_un addr_get;
	memset(&addr_get, 0, sizeof(addr_get));
	addr_get.sun_family = AF_UNIX;
	strncpy(addr_get.sun_path, "/tmp/ytfd.get.sock", sizeof(addr_get.sun_path)-1);

	struct sockaddr_un addr_fetch;
	memset(&addr_fetch, 0, sizeof(addr_fetch));
	addr_fetch.sun_family = AF_UNIX;
	strncpy(addr_fetch.sun_path, "/tmp/ytfd.fetch.sock", sizeof(addr_fetch.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *)&addr_get, sizeof(addr_get)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	int n = write(sockfd, channel_name, strlen(channel_name));	
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	char buf[1024];
	n = read(sockfd, buf, 1024);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	if (n < 1024) {
		buf[n] = '\0';
	}
	if (buf[0]) {
		printf("%s", buf);
		goto finish;
	}

	printf("[NOTE]: not subbed to channel '%s'\n", channel_name);
	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	if (connect(sockfd, (struct sockaddr *)&addr_fetch, sizeof(addr_fetch)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	n = write(sockfd, channel_name, strlen(channel_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	memset(buf, 0, sizeof(buf));
	n = read(sockfd, buf, 1024);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	if (n < 1024) {
		buf[n] = '\0';
	}
	if (buf[0] == 0) {
		fprintf(stderr, "[ERROR]: %s\n", buf+1);
		return EINVAL;
	}
	printf(buf);

finish:
	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int help() {
	printf("TODO:\n");
	return 0;
}

int main(int argc, char **argv) {

	if (argc == 2) {
		if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			return help();
		} else if (strcmp(argv[1], "subs") == 0) {
			return handle_subs();
		} else {
			// NOTE: monitor whether this makes sense
			if (handle_fetch(argv[1]) == 0) {
				return 0;
			}
			return handle_search(argv[1]);
		}
	}

	if (argc != 3) {
		fprintf(stderr, "[ERROR]: invalid number of arguments, expected: <action> <channel>\n");
		return EINVAL;
	}

	if (strcmp(argv[1], "health") == 0) {
		return handle_health(argv[2]);
	} else if (strcmp(argv[1], "search") == 0) {
		return handle_search(argv[2]);
	} else if (strcmp(argv[1], "sub") == 0) {
		if (handle_sub(argv[2]) == 0) {
			return 0;
		}
		return handle_search(argv[2]);
	} else if (strcmp(argv[1], "unsub") == 0) {
		return handle_unsub(argv[2]);
	} else if (strcmp(argv[1], "fetch") == 0 || strcmp(argv[1], "get") == 0) {
		return handle_fetch(argv[2]);
	} else {
		// MAYBE: handle as fetch
		fprintf(stderr, "[ERROR]: unsupported command: %s\n", argv[1]);
		return EINVAL;
	}
	return 0;
}