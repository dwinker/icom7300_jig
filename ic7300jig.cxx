#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "serial.h"
#include "ic7300.h"
#include "scope_waveform_data.h"

static void print_usage_exit(void);

int main(int argc, char **argv)
{
    int c;
    int retval = 0;
    double p_drop_byte = 0.0;

    static const struct option long_options[] =
    {
        {"device",         required_argument, 0, 'd'},
        {"p_drop_byte",    required_argument, 0, 'p'},
        {"changing_scale", no_argument,       0, 's'},
        {0, 0, 0, 0}
    };

    const char *devStr = "/dev/ttyUSB0";

    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (-1 != (c = getopt_long(argc, argv, "d:p:s", long_options, &option_index)))
    {
        switch (c)
        {
            case 'd':
              printf ("option -d with value '%s'\n", optarg);
              devStr = optarg;
              break;
            case 's':
              puts ("option -s");
              g_changing_scale = true;
              break;
            case 'p':
              printf ("option -p with value '%s'\n", optarg);
              p_drop_byte = strtod(optarg, NULL);
              if(0.0 > p_drop_byte || 1.0 < p_drop_byte) {
                  puts("WARNING: probably of dropping a particular byte must");
                  puts("be between 0.0 and 1.0 inclusive. Being set to 0.0.");
                  p_drop_byte = 0.0;
              }
              break;
            case '?':
              /* getopt_long already printed an error message. */
              print_usage_exit();
              break;
            default:
              abort ();
        }
    }

    g_p_drop_byte = (long int)(((double)RAND_MAX + 1.0) * p_drop_byte) - 1;

    (void)serial_init(devStr);

    (void)getchar();

    retval = serial_close();

    return retval;
}

void print_usage_exit(void)
{
    puts("usage: ic7300jig [-d|--device[=]/dev/ttyUSBx] [-p|--p_drop_byte[=]PROB] [-s|--changing_scale]");
    puts("  default device is /dev/ttyUSB0.");
    puts("  PROB is probability that each particular transmit byte will be dropped.");
    puts("  PROB must be between 0.0 and 1.0 inclusive. 0.0 is the default.");
    exit(1);
}
