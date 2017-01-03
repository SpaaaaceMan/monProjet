#include "bor-util.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
	char *fnofam_name, *pipe_li_name, pipe_sc_name[100], pipe_cs_name[100];
	int pip_li, pip_sc, pip_cs;
} Server;

int mainLoop = 1;

void endHandler (int sig) {
	(void) sig;
	mainLoop = 0;
}

int create_pipe_listen (Server *server) {
	
	printf("Opening listening pipe\n");
	
	server->pipe_li_name = "tube-ec-tmp";
	
	int res = mkfifo(server->pipe_li_name, 0600);
	if (res < 0) {
		perror ("mkfifo pipe_li");
		return res;
	}
	
	return 0;
}

int delete_pipe_listen (Server *server) {
	
	printf("Deleting listening pipe\n");
	
	close(server->pip_li);
	unlink(server->pipe_li_name);
}

int wait_contact (Server *server) {
	
	server->pip_li = open(server->pipe_li_name, O_RDONLY);
	if (server->pip_li < 0) {
		perror ("open pip_li");
		return server->pip_li;
	}
	
	char buf [202];

	int res = bor_read_str(server->pip_li, buf, sizeof(buf));
	if (res <= 0) {
		return res;
	}
	
	sscanf(buf, "%s\n%s", server->pipe_sc_name, server->pipe_cs_name);
}

int open_service_pipes(Server *server) {
	
	// Openning
	server->pip_sc = open(server->pipe_sc_name, O_WRONLY);
	if (server->pip_sc < 0) {
		perror ("open pip_li");
		return server->pip_li;
	}
	
	server->pip_cs = open(server->pipe_cs_name, O_RDONLY);
	if (server->pip_cs < 0) {
		close(server->pip_sc);
		perror ("open pip_li");
		return server->pip_li;
	}
}

int close_service_pipes(Server *server) {
	close(server->pip_sc);
	close(server->pip_cs);
}

int search_family_name  (char *file, char *buf) {
	FILE* fileFd = fopen(file, "r");
	
	char bufLine[100];
	char* name;
	char* firstNames;
	
	// Searching in file file the line with the first word equals to first word of buf
	// then memorize in buf the end of line after ':'
	while (fgets(bufLine, sizeof(bufLine), fileFd) != EOF) {
		sscanf(bufLine, "%s%*[^:] %[^\u]", name, firstNames);
		
		if (strcasecmp(name, buf) == 0) {
			break;
		}
		
		firstNames = NULL;
	}
	
	if (firstNames == NULL) {
		firstNames = "not found\n";
	}
	
	buf = firstNames;
	
	return 0;
}

/* Return -1 on error, 0 on EOF, 1 if ok*/
int do_dialog(Server *server) {
	// Reading in pip_cs
	// Search in server->fnofam_name with server_search_family_name()
	// Write answere in pip_sc
	
	char buf[1024];
	
	int readRes = bor_read_str(server->pip_cs, buf, sizeof(buf));
	if (readRes <= 0) {
		return readRes;
	}
	
	int res = search_family_name(server->fnofam_name, buf);
	if (res < 0) {
		fprintf(stderr, "search family name\n");
	}
	
	int writeRes = bor_write_str(server->pip_sc, buf);
	if (writeRes <= 0) {
		return writeRes;
	}
	
	return readRes;
}

void child_main (Server *server) {
	int res;
	
	if (open_service_pipes(server) < 0) {
		exit(1);
	}
	
	while (1) {
		res = do_dialog(server);
		if (res <= 0) {
			break;
		}
	}
	
	printf("End child server\n");
	close_service_pipes(server);
	exit ((res <= -1) ? 1 : 0);
}

int do_Transaction (Server *server) {
	wait_contact(server);
	
	// fork -> closing pip_li then child_main(server)
	
	// father wait end of pip_li with while read > 0
	// closing pip_li
	
	pid_t forkRes = fork();
	if (forkRes < 0) {
		perror("fork");
		return forkRes;
	}
	
	if (forkRes == 0) {
		close(server->pip_li);
		
		int res = child_main(server);
		
		exit(res);
	}
	
	char buf[42];
	while (bor_read(server->pip_li, buf, sizeof(buf) > 0) {}
	close(server->pip_li);
}

int main (int agc, char *argv[]) {

	if (argc - 1 < 1) {
		fprintf(stderr, "1 args waited\n");
	}

	Server server;
	server->fnofam_name = argv[1];
	
	// Parsing args
	if (create_pipe_listen(&server) < 0) {
		exit(1);
	}
	
	// Protection against SIGPIPE
	bor_signal (SIGPIPE, SIG_IGN, SA_RESTART);
	// End server
	bor_signal (SIGINT, endHandler, 0);
	
	// We should also  handle SIGCHLD to get rid of zombies
	while (mainLoop) {
		res = do_transaction(&server);
		if (res <= 0) {
			break;
		}
	}
	
	return 0;
}
