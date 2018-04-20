#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "g_opt.h"
#include "serial.h"
#include "ic7300.h"
#include "scope_waveform_data.h"

static void print_usage_exit(void);

long int g_opt_p_drop_byte;
long int g_opt_p_drop_message;
int      g_opt_n_drop_message = 1;
long int g_opt_p_hesitate;
int      g_opt_t_hesitate = 1000;
int      g_opt_n_hesitate = 1;

int main(int argc, char **argv)
{
    int c;
    int retval = 0;
    double p_drop_byte    = 0.0;
    double p_drop_message = 0.0;
    double p_hesitate     = 0.0;

    static const struct option long_options[] =
    {
        {"device",         required_argument, 0, 'd'},
        {"changing_scale", no_argument,       0, 's'},
        {"p_drop_byte",    required_argument, 0, 'p'},
        {"p_drop_message", required_argument, 0, 'm'},
        {"n_drop_message", required_argument, 0, 'n'},
        {"p_hesitate",     required_argument, 0, 'h'},
        {"t_hesitate",     required_argument, 0, 't'},
        {"n_hesitate",     required_argument, 0, 'k'},
        {0, 0, 0, 0}
    };

    const char *devStr = "/dev/ttyUSB0";

    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (-1 != (c = getopt_long(argc, argv, "d:sp:m:n:h:t:k:", long_options, &option_index)))
    {
        switch (c)
        {
            case 'd':
              printf ("option -d|--device with value '%s'\n", optarg);
              devStr = optarg;
              break;
            case 's':
              puts ("option -s|--changing_scale");
              g_changing_scale = true;
              break;
            case 'p':
              printf ("option -p|--p_drop_byte with value '%s'\n", optarg);
              p_drop_byte = strtod(optarg, NULL);
              if(0.0 > p_drop_byte || 1.0 < p_drop_byte) {
                  puts("WARNING: probably of dropping a particular byte must");
                  puts("be between 0.0 and 1.0 inclusive. Being set to 0.0.");
                  p_drop_byte = 0.0;
              }
              break;
            case 'm':
              printf ("option -m|--p_drop_message with value '%s'\n", optarg);
              p_drop_message = strtod(optarg, NULL);
              if(0.0 > p_drop_message || 1.0 < p_drop_message) {
                  puts("WARNING: probably of dropping messages must be");
                  puts("between 0.0 and 1.0 inclusive. Being set to 0.0.");
                  p_drop_message = 0.0;
              }
              break;
            case 'n':
              printf ("option -n|--n_drop_message with value '%s'\n", optarg);
              g_opt_n_drop_message = atoi(optarg);
              break;
            case 'h':
              printf ("option -h|--p_hesitate with value '%s'\n", optarg);
              p_hesitate = strtod(optarg, NULL);
              if(0.0 > p_hesitate || 1.0 < p_hesitate) {
                  puts("WARNING: probably of hesitating must be between");
                  puts("0.0 and 1.0 inclusive. Being set to 0.0.");
                  p_hesitate = 0.0;
              }
              break;
            case 't':
              printf ("option -t|--t_hesitate with value '%s'\n", optarg);
              g_opt_t_hesitate = atoi(optarg);
              break;
            case 'k':
              printf ("option -k|--n_hesitate with value '%s'\n", optarg);
              g_opt_n_hesitate = atoi(optarg);
              break;
            case '?':
              /* getopt_long already printed an error message. */
              print_usage_exit();
              break;
            default:
              abort ();
        }
    }

    g_opt_p_drop_byte    = (long int)(((double)RAND_MAX + 1.0) * p_drop_byte)    - 1;
    g_opt_p_drop_message = (long int)(((double)RAND_MAX + 1.0) * p_drop_message) - 1;
    g_opt_p_hesitate     = (long int)(((double)RAND_MAX + 1.0) * p_hesitate)     - 1;

    (void)serial_init(devStr);

    (void)getchar();

    retval = serial_close();

    return retval;
}

void print_usage_exit(void)
{
    puts("usage: ic7300jig [-d|--device[=]/dev/ttyUSBx] [-s|--changing_scale] [ERROR INJECTION OPTIONS]");
    puts("  default device is /dev/ttyUSB0.");
    puts("  Changing scale refers to the scale on the remote scope.");
    puts("  The following optional ERROR INJECTION OPTIONS can be given to cause timeouts of various kinds on the other end:");
    puts("  -p|--p_drop_byte   [=]PROB probability that each particular byte will be dropped.");
    puts("  -m|--p_drop_message[=]PROB probability that message dropping will be triggered.");
    puts("  -n|--n_drop_message[=]N    number of messages that will be dropped once triggered by -m.");
    puts("  -h|--p_hesitate    [=]PROB probablilty that message hesitation will be triggered.");
    puts("  -t|--t_hesitate    [=]T    number of milleseconds to hesitate when triggered by -h.");
    puts("  -k|--n_hesitate    [=]N    number of messages that will be hesitated once triggered by -h.");
    puts("  PROB must be between 0.0 and 1.0 inclusive. 0.0 is the default.");
    puts("  T is an integer. 1000 is used if option is not given.");
    puts("  N is an integer. 1 is used if option is not given.");
    exit(1);
}
