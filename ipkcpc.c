#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define SUCCESS 0
#define MAX_HOST_SIZE 10
#define MAX_PORT_SIZE 15 //IPv4 max length

const char * usage = "ipkcpc -h <host> -p <port> -m <mode>";

typedef enum mode_t {
    undefined,
    tcp,
    udp
}mode_t;

void print_mode(mode_t m)
{
    if(m == tcp) printf("tcp\n");
    else if(m == udp) printf("udp\n");
    else printf("undefined\n");
}

bool has_value(char * indicator, const char * msg)
{
    if(indicator == NULL || *indicator == '-')
    {
        fprintf(stderr,"%s\n",msg);
        return false;
    }
    return true;
}

/**
 * @brief Nastavi mod spojenia (protokol)
 * 
 * @param mode - parameter 
 * @return mode_t 
 */
mode_t set_mode(char ** mode)
{
    if(!strcmp("tcp",*mode)) return tcp;
    else if(!strcmp("udp",*mode)) return udp;
    else return undefined;
}

int main(int argc, char **argv)
{
    //basic params to work with
    char * host= NULL;
    char * port= NULL;
    mode_t mode = undefined;

    //zaciatok command line args
    for(int arg_num = 1; arg_num < argc; arg_num++)
    {
        //prepinac detekovany
        if(argv[arg_num][0] == '-')
        {
            switch (argv[arg_num][1])
            {
                //host
                case 'h': 
                    if(!has_value(argv[++arg_num],"HOST expected"))
                    {
                        arg_num -= 1;
                        break;
                    }
                    //host check 
                    host = argv[arg_num];
                    break;
                //port
                case 'p':
                    if(!has_value(argv[++arg_num],"PORT expected"))
                    {
                        arg_num -= 1;
                        break;
                    }
                    //port check
                    port = argv[arg_num];
                    break;
                //mode
                case 'm':
                    if(!has_value(argv[++arg_num],"MODE expected"))
                    {
                        arg_num -= 1;
                        break;
                    }
                    mode = set_mode(&argv[arg_num]);
                    break;
                default:
                    fprintf(stderr,"Error: Unknown option.\n");
                    break;
            }
        }
        else fprintf(stderr,"Error: Unknown command line arg.\n");
    }

    printf("\n-----GOT DATA-----\n");
    if(host) printf("HOST: %s\n",host);
    if(port) printf("PORT: %s\n",port);
    printf("MODE: "); print_mode(mode);
    return SUCCESS;
}
