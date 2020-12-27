#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <json-c/json.h>


#define PORT 8080

#define MAXPENDING 5
#define BUFFSIZE 2048
#define MAX 2048

void Die(char *mess) 
{
    perror(mess);
    exit(1);
}

void setup(char inputBuffer[], char *args[], int *background) //Prepares command
{
        const char s[4] = " \t\n";
        char *token;
        token = strtok(inputBuffer, s);
        int i = 0;
        while( token != NULL)
        {
            args[i] = token;
            i++;
            //printf("%s\n", token);
            token = strtok(NULL,s);
        }
        args[i] = NULL;
}

void HandeClient(int sock){

    char buffer[BUFFSIZE];
    int received = -1;
    char data[MAX];
    memset(data,0,MAX);

    while(1) { // this will make server wait for another command to run until it receives exit
        data[0] = '\0';
        if((received = recv(sock, buffer,BUFFSIZE,0))<0){

            Die("Failed");
        }

        buffer[received] = '\0';

        strcat (data,  buffer);
        if (strcmp(data, "exit")==0) // this will force the code to exit
            exit(0);

        puts (data);
        char *args[100];
        setup(data,args,0);
        int pipefd[2],lenght;

        char *fake_args[2];
        fake_args[0] = "ls";
        fake_args[1] = "cat";

        if(pipe(pipefd))
            Die("Failed to create pipe");

        pid_t pid = fork();
        char path[MAX];

        if(pid==0)
        {
            close(1); // close the original stdout
            dup2(pipefd[1],1); // duplicate pipfd[1] to stdout
            close(pipefd[0]); // close the readonly side of the pipe
            close(pipefd[1]); // close the original write side of the pipe
            char *arguments[] = { fake_args[0], NULL };
            execvp(fake_args[0],arguments); // finally execute the command
        }
        else
            if(pid>0)
            {
                close(pipefd[1]);
                memset(path,0,MAX);
                while(lenght=read(pipefd[0],path,MAX-1)){
                    printf("Data read so far %s\n", path);
                    if(send(sock,path,strlen(path),0) != strlen(path) ){
                        Die("Failed");
                    }
                    fflush(NULL);
                    printf("Data sent so far %s\n", path);
                memset(path,0,MAX);
                }
                close(pipefd[0]);
                //exit(1); removed so server will not terminate
            }
            else
            {
                printf("Error !\n");
                exit(0);//
            }
    }
}

int get_file_contents(const char* filename, char** outbuffer) {
  FILE* file = NULL;
  long filesize;
  const int blocksize = 1;
  size_t readsize;
  char* filebuffer;

  // Open the file
  file = fopen(filename, "r");
  if (NULL == file)
  {
    printf("'%s' not opened\n", filename);
    exit(EXIT_FAILURE);
  }

  // Determine the file size
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  rewind (file);

  // Allocate memory for the file contents
  filebuffer = (char*) malloc(sizeof(char) * filesize);
  *outbuffer = filebuffer;
  if (filebuffer == NULL)
  {
    fputs ("malloc out-of-memory", stderr);
    exit(EXIT_FAILURE);
  }

  // Read in the file
  readsize = fread(filebuffer, blocksize, filesize, file);
  if (readsize != filesize)
  {
    fputs ("didn't read file completely",stderr);
    exit(EXIT_FAILURE);
  }

  // Clean exit
  fclose(file);
  return EXIT_SUCCESS;
}

int ParseTest(const char *file){
    // Install instructions for json-c library: https://github.com/json-c/json-c
    char* json = NULL;                                                             
    struct json_object *obj;                                                    
    struct stat st;                                                             

    get_file_contents(file, &json);
	// printf("%s",json);

    obj = json_tokener_parse(json);                                     

    json_object_object_foreach(obj, key, val) {                                 
        printf("key = %s value = %s\n",key, json_object_get_string(val));       
    }
}


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, bytes_read;
    struct sockaddr_in address;
    struct sockaddr_in clnt_address;
    int clnt_addrlen = sizeof(clnt_address);
    const int BUF_LEN = 256;
    char buffer[BUF_LEN];

    char file_name[] = "src/example.json";
    ParseTest(file_name);

    return 0;

    memset(buffer, 0, BUF_LEN);

    /* Create socket file descriptor */
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    // Attach socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    { 
        // accept a connection on a socket
        if ((new_socket = accept(server_fd, (struct sockaddr *) &clnt_address, (socklen_t*) &clnt_addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        HandeClient(new_socket);
        /* Read network pipe data */
        // while ((bytes_read = read(new_socket, buffer, BUF_LEN)) > 0) {
        //     printf("%s", buffer);
        // }
        // printf("\n");

        close(new_socket);
    };

    return 0;
}
