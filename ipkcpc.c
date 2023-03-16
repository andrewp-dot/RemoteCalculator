#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#if defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION) || defined(__APPLE__)

#endif

#define SUCCESS 0
#define ERROR_ARGUMENTS 1 

/**
 * TODO:
 * dorobit usage + error kody
 * napisat testy a fixnut kontrolu IPv4 adresy 
 */
#define USAGE "./ipkcpc -h <host> -p <port> -m <mode>\n"
#define MAX_IP_PART_SIZE 255
#define MAX_PORT 65535

typedef enum connection {
    undefined,
    tcp,
    udp
}connection_t;

// struct udp_addr_in
// {
//     /* data */
// };

// struct udp_addr_res
// {
//     /* data */
// };



void print_mode(connection_t m);

/**
 * @brief Nastavi mod spojenia (protokol)
 * 
 * @param mode - parameter 
 * @return mode_t 
 */
connection_t set_mode(char ** mode);

bool has_value(char * indicator);
bool is_num(char * str);
bool ip_is_ok(char * host);

int udp_connection();
int tcp_connection();

void copy_after_payload(char * dest, char * src)
{
    dest[0] = 0;
    for (int i = 1; i < strlen(src); i++)
    {
        dest[i] = src[i-1];
    }
    
}

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
    connection_t mode = undefined;

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
    if(mode == tcp) return tcp_connection();
    return SUCCESS;
}

void print_mode(connection_t m)
{
    if(m == tcp) printf("tcp\n");
    else if(m == udp) printf("udp\n");
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

int udp_connection(int port, char * host)
{
    fprintf(stdout,"PORT %d\n",port);
    int family = AF_INET;
    // int type = SOCK_DGRAM;

    //create socket
    int client_socket = socket(family, SOCK_DGRAM, 0); 
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

    //problem s req formatom - opcode + payload length + payload
    int flags = 0;
    char buffer[1024];

    fgets(buffer, 1023, stdin);
    char msg_buffer[1024] = {0};
    msg_buffer[0] = 0;
    copy_after_payload(msg_buffer,buffer);

    //send message - tu bude asi zacinat while -> fgets -> sendto
    if(!strcmp("exit",msg_buffer))
    {
        close(client_socket);
        return SUCCESS;
    }
    //strlen hadze chybu lebo zacina 0
    int bytes_tx = sendto(client_socket, msg_buffer, strlen(msg_buffer),flags, (struct sockaddr *) &server_address, sizeof(server_address));
    printf("[+] Data sent: %s\n",msg_buffer + 1);
    if(bytes_tx < 0)
    {
        fprintf(stderr,"Error: sendto\n");
        return EXIT_FAILURE;
    }

    bzero(msg_buffer,1024);

    //rec from
    socklen_t rec_addr = sizeof(server_address);
    int bytes_rx = recvfrom(client_socket, msg_buffer, 1024, flags, (struct sockaddr *)&server_address, &rec_addr);
    if (bytes_rx < 0)
    {
        fprintf(stderr,"ERROR: recvfrom"); 
    }
    printf("%s\n",msg_buffer);
    close(client_socket);
    return SUCCESS;
    }

int tcp_connection()
{
    printf("TCP\n");
    return SUCCESS;
}
