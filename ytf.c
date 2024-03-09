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

int calc_buf_size(char pre_buf[3]) {
	int buf_size = 2;
	int pow = pre_buf[1]*10+pre_buf[2];
	for (int i=0; i<pow-1; ++i) {
		buf_size *= 2;
	}
	return buf_size;
}

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

	char pre_buf[3];
	n = read(sockfd, pre_buf, 3);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	
	int msg_size = calc_buf_size(pre_buf);
	char buf[msg_size];
	n = read(sockfd, buf, msg_size);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	buf[n] = '\0';
	printf("%s", buf);
	if (!pre_buf[0]) {
		status = EADDRINUSE; // TODO: better error code
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

	// TODO: fix
	char pre_buf[3];
	n = read(sockfd, pre_buf, 3);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	printf("%d %d %d", pre_buf[0], pre_buf[1], pre_buf[2]);
	if (pre_buf[0]) {
		int msg_size = (pre_buf[1]-'0')*10+(pre_buf[2]-'0');
		printf("%d\n", msg_size);
		char buf[msg_size];
		n = read(sockfd, buf, msg_size);
		if (n == -1) {
			close(sockfd);
			fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
			return errno;
		}
		if (n != msg_size) {
			fprintf(stderr, "[WARN]: read incorrect nr of bytes: %d vs %d\n", n, msg_size);
		}
		printf("%s", buf);
		goto finish;
	}
	return 0;

	char buf[1024]; // REMOVEME:

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
		// return handle_search(argv[2]);
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