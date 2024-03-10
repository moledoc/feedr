#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// TODO: beter logging
// TODO: help
// TODO: readme

int calc_buf_size(char pre_buf[3]) {
	int buf_size = 2;
	int pow = pre_buf[1]*10+pre_buf[2];
	for (int i=0; i<pow-1; ++i) {
		buf_size *= 2;
	}
	return buf_size;
}

int prep_conn(char *path, int *fd) {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: failed to connect to socket: %s\n", strerror(errno));
		return errno;
	}
	*fd = sockfd;
	return 0;
}

int handle_response(int sockfd) {
	char pre_buf[3];
	int n = read(sockfd, pre_buf, 3);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	int buf_size = calc_buf_size(pre_buf);
	char buf[buf_size+1];
	n = read(sockfd, buf, buf_size);
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	buf[n] = '\0';
	printf("%s\n", buf);
	return pre_buf[0];
}

int handle_health(char *msg) {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.health.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	int n = write(sockfd, msg, strlen(msg));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	handle_response(sockfd);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[WARNING]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_sub(char *chan_name) {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.add.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	int n = write(sockfd, chan_name, strlen(chan_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	int successful = handle_response(sockfd);
	if (!successful) {
		return EADDRINUSE; // TODO: better error code
	}

	return 0;		
}

int handle_search(char *channel_name) {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.search.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	int n = write(sockfd, channel_name, strlen(channel_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	handle_response(sockfd);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: close search: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_unsub(char *channel_name) {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.rm.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	int n = write(sockfd, channel_name, strlen(channel_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	handle_response(sockfd);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_subs() {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.subs.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	handle_response(sockfd);

	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int handle_fetch(char *channel_name) {
	int sockfd;
	int err = prep_conn("/tmp/ytfd.get.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	int n = write(sockfd, channel_name, strlen(channel_name));	
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	int successful = handle_response(sockfd);
	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	if (successful) {
		return 0;
	}

	err = prep_conn("/tmp/ytfd.fetch.sock", &sockfd);
	if (err != 0) {
		return err;
	}

	n = write(sockfd, channel_name, strlen(channel_name));
	if (n == -1) {
		close(sockfd);
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}

	handle_response(sockfd);
	if (close(sockfd) == -1) {
		fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int help(int argc, char **argv) {
	printf("%s <action|channel>\n", argv[0]);
	printf("ACTION:\n");
	printf("\t<channel>\t\t\t\tget latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube\n");
	printf("\thelp, -h, --help\t\tprint help\n");
	printf("\thealth [message]\t\tchecks daemon health; if message given, daemon reflects it, otherwise responds with pre-set message\n");
	printf("\tsubs\t\t\t\t\tprint list of subscribed channels\n");
	printf("\tsearch <channel>\t\tsearch channel; partial search somewhat effective, search with typo not so much\n");
	printf("\tsub <channel>\t\t\tsubscribe to channel\n");
	printf("\tunsub <channel>\t\t\tunsubscribe to channel\n");
	printf("\tfetch <channel>\t\t\tget latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube\n");
	printf("\tget <channel>\t\t\tget latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube\n");
	printf("EXAMPLES:\n");
	printf("\t* %s health\n", argv[0]);
	printf("\t* %s health 'hello world'\n", argv[0]);
	printf("\t* %s subs\n", argv[0]);
	printf("\t* %s tsodingdaily\n", argv[0]);
	printf("\t* %s search ThePrimea\n", argv[0]);
	printf("\t* %s sub ThePrimeTimeagen\n", argv[0]);
	printf("\t* %s unsub ThePrimeTimeagen\n", argv[0]);
	printf("\t* %s fetch TsodingDaily\n", argv[0]);
	printf("\t* %s get tsodingdaily\n", argv[0]);
	return 0;
}

int main(int argc, char **argv) {
	if (argc == 2) {
		if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			return help(argc, argv);
		} else if (strcmp(argv[1], "health") == 0) {
			return handle_health("I'm! alive!!! muhahahaha!");
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
		return handle_sub(argv[2]);
	} else if (strcmp(argv[1], "unsub") == 0) {
		return handle_unsub(argv[2]);
	} else if (strcmp(argv[1], "fetch") == 0 || strcmp(argv[1], "get") == 0) {
		return handle_fetch(argv[2]);
	} else {
		fprintf(stderr, "[ERROR]: unsupported command: %s\n", argv[1]);
		return EINVAL;
	}
	return 0;
}