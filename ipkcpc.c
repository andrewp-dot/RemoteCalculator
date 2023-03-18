#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#if !defined(_WIN32) && (defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION) || defined(__APPLE__))
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#endif

#define SUCCESS 0
#define ERROR_ARGUMENTS 1 

/**
 * TODO:
 * dorobit usage + error kody
 * testy 
 * kompatibilitu pre windows
 */
#define USAGE "./ipkcpc -h <host> -p <port> -m <mode>\n"
#define MAX_IP_PART_SIZE 255
#define MAX_PORT 65535
#define BUFFER_SIZE 1024

typedef enum connection {
    undefined,
    tcp,
    udp
}connection_t;

//nahradit prepinacom
int * g_socket_pointer = false;
connection_t mode = undefined; 
void close_connection(int num);

void print_mode();

/**
 * @brief Nastavi mod spojenia (protokol)
 * 
 * @param mode - parameter 
 * @return connection_t 
 */
connection_t set_mode(char ** mode);

bool has_value(char * indicator);
bool is_num(char * str);
bool ip_is_ok(char * host);

int udp_connection();
int tcp_connection();

int main(int argc, char **argv)
{

    if(argc <= 1)
    {
        fprintf(stderr,USAGE);
        return ERROR_ARGUMENTS;
    }

    //basic params to work with
    char * host= NULL;
    char * port= NULL;

    //zaciatok command line args
    for(int arg_num = 1; arg_num < argc; arg_num++)
    {
        if(argv[arg_num][0] != '-')
        {
            fprintf(stderr,"Error: Unknown command line arg.\nUsage: %s", USAGE);
            return ERROR_ARGUMENTS;
        }

        switch (argv[arg_num][1])
        {
            //host
            case 'h': 
                if(!has_value(argv[++arg_num]) || host != NULL || !ip_is_ok(argv[arg_num]))
                {
                    fprintf(stderr,"Usage: %s",USAGE);
                    return ERROR_ARGUMENTS;
                } 

                host = argv[arg_num]; 
                break;
            //port
            case 'p':
                if(!has_value(argv[++arg_num]) || port != NULL || !is_num(argv[arg_num]) || atoi(argv[arg_num]) > MAX_PORT) 
                {
                    fprintf(stderr,USAGE);
                    return ERROR_ARGUMENTS;
                }
                port = argv[arg_num];
                break;
            //mode
            case 'm':
                if(!has_value(argv[++arg_num]) || mode != undefined)
                {
                    fprintf(stderr,USAGE);
                    return ERROR_ARGUMENTS;
                }
                mode = set_mode(&argv[arg_num]);
                break;
            default:
                fprintf(stderr,"Error: Unknown option.\n");
                break;
        }
    }

    if(mode == udp) return udp_connection(atoi(port),host);
    if(mode == tcp) return tcp_connection(atoi(port),host);
    return SUCCESS;
}

void print_mode()
{
    if(mode == tcp) printf("tcp\n");
    else if(mode == udp) printf("udp\n");
    else printf("undefined\n");
}

connection_t set_mode(char ** mode)
{
    if(!strcmp("tcp",*mode)) return tcp;
    else if(!strcmp("udp",*mode)) return udp;
    else return undefined;
}

bool has_value(char * indicator)
{
    if(indicator == NULL || *indicator == '-') return false;
    return true;
}

bool is_num(char * str)
{
    while (*str)
    {
        if(!isdigit(*str)) return false;
        str++;
    }
    return true;
}

bool ip_is_ok(char * host)
{
    int ip_length = strlen(host);
    if(!isdigit(*host) || !isdigit(host[ip_length-1])) return false;
    char verify_host[ip_length];
    strcpy(verify_host,host);
    char * token = strtok(verify_host,".");
    short int token_num = 0;
    while (token != NULL)
    {
        if(token_num > 3 || !is_num(token) || atoi(token) > MAX_IP_PART_SIZE) return false;
        token = strtok(NULL,".");
        token_num += 1;
    }
    if(token_num != 4 ) return false;
    return true;
}

void close_connection(int num)
{
    //ak nebol vytvoreny ziadny socket
    if(g_socket_pointer == NULL) exit(SUCCESS);
    if(mode == tcp)
    {
        char buffer[BUFFER_SIZE];
        write(STDOUT_FILENO,"BYE\n",5);
        int bytes_tx = send(*g_socket_pointer,"BYE\n",strlen("BYE\n"),0);
        if (bytes_tx < 0)
        {
            write(STDERR_FILENO,"ERROR: send",12);
        }

        memset(&buffer, 0, strlen(buffer));

        int bytes_rx = recv(*g_socket_pointer,buffer,BUFFER_SIZE,0);
        if (bytes_rx < 0)
        {
            write(STDERR_FILENO,"ERROR: recv",12);
        }

        write(STDIN_FILENO,buffer,strlen(buffer));

        shutdown(*g_socket_pointer,SHUT_RDWR);
        g_socket_pointer = NULL;
        exit(SUCCESS);
    }
    else if(mode == udp)
    {
        if(close(*g_socket_pointer)) exit(EXIT_FAILURE);
        exit(SUCCESS);
    }
}

int udp_connection(int port, char * host)
{
    //handling terminating
    signal(SIGINT,close_connection);
    signal(SIGTERM,close_connection);

    int family = AF_INET;

    //create socket
    int client_socket = socket(family, SOCK_DGRAM, 0); 
    g_socket_pointer = &client_socket;
    if (client_socket <= 0)
    {
        fprintf(stderr, "ERROR: socket");
        return EXIT_FAILURE;
    }

    //setup server addres
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = family;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);
    
    printf("INFO: Server socket: %s : %d \n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    int flags = 0;
    char buffer[BUFFER_SIZE];

    while (true)
    {
        fgets(buffer, BUFFER_SIZE-2, stdin);
        char msg_buffer[BUFFER_SIZE] = {0};
        msg_buffer[0] = 0;
        msg_buffer[1] = strlen(buffer);
        
        strcpy(msg_buffer+2,buffer);
        if(msg_buffer[msg_buffer[1]+1] == '\n') //potencionalny koniec riadku na indexe msg_buffer[msg_bffer[1]-1+2]
        {
            msg_buffer[msg_buffer[1]+1] = 0;
            msg_buffer[1] -= 1;
        }

        //send message 
        if(!strcmp("exit",msg_buffer + 2))
        {
            close(client_socket);
            return SUCCESS;
        }
        
        int bytes_tx = sendto(client_socket, msg_buffer, msg_buffer[1] + 2,flags, (struct sockaddr *) &server_address, sizeof(server_address));
        if(bytes_tx < 0)
        {
            fprintf(stderr,"Error: sendto\n");
            return EXIT_FAILURE;
        }

        memset(&msg_buffer, 0, strlen(buffer));

        //rec from
        socklen_t rec_addr = sizeof(server_address);
        int bytes_rx = recvfrom(client_socket, msg_buffer, BUFFER_SIZE, flags, (struct sockaddr *)&server_address, &rec_addr);
        if (bytes_rx < 0)
        {
            fprintf(stderr,"ERROR: recvfrom");
            continue;
        }
        if(msg_buffer[1]) printf("ERR:");
        else printf("OK:");

        printf("%s",msg_buffer + 3);
    }
    
    close(client_socket);
    return SUCCESS;
}

int tcp_connection(int port, char * host)
{   
     //handling terminating
    signal(SIGINT,close_connection);
    signal(SIGTERM,close_connection);

    int family = AF_INET;

    int client_socket = socket(family, SOCK_STREAM, 0); 
    g_socket_pointer = &client_socket;
    if (client_socket <= 0)
    {
        fprintf(stderr, "ERROR: socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = family;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);
    socklen_t server_addr_size = sizeof(server_address);

    if(connect(client_socket,(const struct sockaddr *)&server_address,server_addr_size) != 0)
    {
        fprintf(stderr,"Error: connection failed.\n");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    bool end_connection = false;

    while (true)
    {
        fgets(buffer, BUFFER_SIZE-2, stdin);
        if(!strcmp("BYE\n",buffer)) end_connection = true;

        int bytes_tx = send(client_socket,buffer,strlen(buffer),0);
        if (bytes_tx < 0)
        {
            fprintf(stderr,"ERROR: send");
        }

        memset(&buffer, 0, strlen(buffer));

        int bytes_rx = recv(client_socket,buffer,BUFFER_SIZE,0);
        if (bytes_rx < 0)
        {
            fprintf(stderr,"ERROR: recv");
        }
        printf("%s",buffer);
        if(end_connection)
        {
            shutdown(client_socket,SHUT_RDWR);
            return SUCCESS;
        }
        
    }

    return SUCCESS;
}
