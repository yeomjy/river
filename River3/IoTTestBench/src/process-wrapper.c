#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define PORT 8080

int connect_to_server()
{
    int server_sock;
    struct sockaddr_in server_addr;

    if ((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed! %s\n", strerror(errno));
        return -1;
    }

    return server_sock;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;
    int pipefd[2];
    int bytes_read;
    const int BUF_LEN = 256;
    char buf[BUF_LEN];

    int server_sock= 0;

    /* Build command vector as argv[1 .. argc] */
    char** cmdv = (char**) malloc (argc * sizeof(char*));
    char** p = &argv[1];
    int idx = 0;

    while (*p) {
        cmdv[idx++] = *p;
        p++;
    }
    cmdv[idx] = NULL;

    /* Open communication pipe */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    switch (pid) {
        case -1:
            /* error forking */
            return EXIT_FAILURE;
        case 0:
            close(pipefd[0]); /* Close unuser read end */
            dup2(pipefd[1], STDOUT_FILENO); /* Redirect childs stdout to parent pipe */
            /* Execute child process */
            execvp(cmdv[0], (char *const *) cmdv);

            /* only if exec failed */
            exit(EXIT_FAILURE);
        default:
            /* parent process */
            break;
    }

    /* only parent process gets here */

    close(pipefd[1]); /* Close unused write end */

    /* wait for child process to finish */
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        printf("Child %d terminated normally, with code %d\n",
                pid, WEXITSTATUS(status));

    server_sock = connect_to_server();

    /* Send child process output to server, thus simulating a network pipe */
    while ((bytes_read = read(pipefd[0], &buf, BUF_LEN)) > 0) {
        send(server_sock, buf, bytes_read, 0);
    }
    close(pipefd[0]); /* Close read end after we're done */

    free(cmdv);

    return status;
}
