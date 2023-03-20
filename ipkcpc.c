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

#define TCP_SHUTDOWN SHUT_RDWR
#define TERMINATION_HANDLING  \
    signal(SIGINT,close_connection); \
    signal(SIGTERM,close_connection);

#define WSA_CLEANUP ;

#elif defined(_WIN32)
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#define TCP_SHUTDOWN SD_BOTH
#define TERMINATION_HANDLING  \
    SetConsoleCtrlHandler(HandlerRoutine,true);

#define WSA_CLEANUP WSACleanup();

#endif

#define SUCCESS 0
#define ERROR_ARGUMENTS 10 
#define BZERO(buffer) memset(buffer, 0, sizeof(buffer));

#define USAGE \
"./ipkcpc -h <host> -p <port> -m <mode>\n\
<host> 0.0.0.0 - 255.255.255.255\n\
<port> 0 - 65535\n\
<mode> udp/tcp\n\
"
#define MAX_IP_PART_SIZE 255
#define MAX_PORT 65535
#define TCP_BUFFER_LIMIT 65536 //2^16
#define UDP_BUFFER_LIMIT 256

#define UDP_HEADER_OFFSET 2
#define UDP_RESPONSE_OFFSET 3
#define END_OF_STRING 1

typedef enum connection {
    undefined,
    tcp,
    udp
}connection_t;

int * g_socket_pointer = NULL;
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

#if defined(_WIN32)
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    if(dwCtrlType == CTRL_C_EVENT)
    {
        close_connection(CTRL_C_EVENT);
        return true;
    } 
    return false;
}
#endif

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
    int ip_length = strlen(host) + END_OF_STRING;
    if(!isdigit(*host) || !isdigit(host[ip_length-END_OF_STRING-1])) return false;
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
        char buffer[TCP_BUFFER_LIMIT];
        if(write(STDOUT_FILENO,"BYE\n",5) < 0) fprintf(stdout,"BYE\n");
        int bytes_tx = send(*g_socket_pointer,"BYE\n",strlen("BYE\n"),0);
        if (bytes_tx < 0)
        {
            if(write(STDERR_FILENO,"ERROR: send",12)< 0) fprintf(stderr,"ERROR: send\n");
        }

        BZERO(buffer)

        int bytes_rx = recv(*g_socket_pointer,buffer,TCP_BUFFER_LIMIT,0);
        if (bytes_rx < 0)
        {
            if(write(STDERR_FILENO,"ERROR: recv",12) < 0)  fprintf(stderr,"ERROR: recv");
        }

        if(write(STDIN_FILENO,buffer,strlen(buffer)) < 0) fprintf(stdout,"%s",buffer);

        shutdown(*g_socket_pointer,TCP_SHUTDOWN);
        if(close(*g_socket_pointer)) exit(EXIT_FAILURE);
        g_socket_pointer = NULL;
        WSA_CLEANUP
        exit(SUCCESS);
    }
    else if(mode == udp)
    {
        if(close(*g_socket_pointer)) 
        {
            WSA_CLEANUP
            exit(EXIT_FAILURE);
        }
        WSA_CLEANUP
        exit(SUCCESS);
    }
}

int udp_connection(int port, char * host)
{
    TERMINATION_HANDLING

    int family = AF_INET;

    #if defined(_WIN32)
    //initializing winsock for windows
    WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
        close_connection(0);
		return EXIT_FAILURE;
	}
	
    #endif

    //create socket
    int client_socket = socket(family, SOCK_DGRAM, 0);
    g_socket_pointer = &client_socket;
    if (client_socket <= 0)
    {
        fprintf(stderr, "ERROR: socket");
        close_connection(0);
        return EXIT_FAILURE;
    }

    //setup server addres
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = family;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);

    int flags = 0;
    char buffer[UDP_BUFFER_LIMIT];
    BZERO(buffer)

    while ( fgets(buffer, UDP_BUFFER_LIMIT-UDP_HEADER_OFFSET, stdin)) 
    {
        char msg_buffer[UDP_BUFFER_LIMIT] = {0};
        msg_buffer[0] = 0;
        msg_buffer[1] = strlen(buffer);
        
        strcpy(msg_buffer+UDP_HEADER_OFFSET,buffer);
        if(msg_buffer[msg_buffer[1]+UDP_HEADER_OFFSET-1] == '\n') 
        {
            msg_buffer[msg_buffer[1]+UDP_HEADER_OFFSET-1] = 0;
            msg_buffer[1] -= 1;
        }

        //send message 
        int bytes_tx = sendto(client_socket, msg_buffer, msg_buffer[1] + UDP_HEADER_OFFSET,flags, (struct sockaddr *) &server_address, sizeof(server_address));
        if(bytes_tx == 0) break;
        if(bytes_tx < 0)
        {
            fprintf(stderr,"Error: sendto\n");
            return EXIT_FAILURE;
        }
        
        BZERO(msg_buffer)

        //rec from
        socklen_t rec_addr = sizeof(server_address);
        int bytes_rx = recvfrom(client_socket, msg_buffer, UDP_BUFFER_LIMIT, flags, (struct sockaddr *)&server_address, &rec_addr);
        if(bytes_rx == 0) break;
        if (bytes_rx < 0)
        {
            fprintf(stderr,"ERROR: recvfrom");
            continue;
        }
        
        if(msg_buffer[1]) printf("ERR:");
        else printf("OK:");

        printf("%s\n",msg_buffer + UDP_RESPONSE_OFFSET);
        BZERO(msg_buffer)
    }
    g_socket_pointer = NULL;
    close(client_socket);
    return SUCCESS;
}

int tcp_connection(int port, char * host)
{   
     //handling terminating
    TERMINATION_HANDLING

    int family = AF_INET;

    #if defined(_WIN32)
    
    WSADATA wsa;
	// printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
    #endif 

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
        fprintf(stderr,"Error: connection failed.");
        return EXIT_FAILURE;
    }

    char send_buffer[TCP_BUFFER_LIMIT];
    char rec_buffer[TCP_BUFFER_LIMIT];
    bool end_connection = false;

    BZERO(send_buffer)
    BZERO(rec_buffer)

    while (fgets(send_buffer, TCP_BUFFER_LIMIT, stdin))
    {
        if(!strncmp("BYE", send_buffer,3)) end_connection = true;

        
        int bytes_tx = send(client_socket,send_buffer,strlen(send_buffer),0);
        if(bytes_tx == 0) break;
        if (bytes_tx < 0)
        {
            fprintf(stderr,"ERROR: send");
        }

        BZERO(send_buffer)
        BZERO(rec_buffer)

        int bytes_rx = recv(client_socket,rec_buffer,TCP_BUFFER_LIMIT,0);
        if(bytes_rx == 0) break;
        if (bytes_rx < 0)
        {
            fprintf(stderr,"ERROR: recv");
        }
        else fprintf(stdout,"%s",rec_buffer);
        
        

        if(!strncmp("BYE", rec_buffer,3)) end_connection = true;
        BZERO(rec_buffer)

        if(end_connection)
        {
            shutdown(client_socket,TCP_SHUTDOWN);
            if(close(client_socket)) return EXIT_FAILURE;
            return SUCCESS;
        }
        
    }
    close_connection(0);
    return SUCCESS;
}
