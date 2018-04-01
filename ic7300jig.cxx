#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "serial.h"
#include "scope_waveform_data.h"

static void print_usage_exit(void);

int main(int argc, char **argv)
{
    int c;
    int retval = 0;

    static const struct option long_options[] =
    {
        {"device",         required_argument, 0, 'd'},
        {"changing_scale", no_argument,       0, 's'},
        {0, 0, 0, 0}
    };

    const char *devStr = "/dev/ttyUSB0";

    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (-1 != (c = getopt_long(argc, argv, "d:s", long_options, &option_index)))
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
            case '?':
              /* getopt_long already printed an error message. */
              print_usage_exit();
              break;
            default:
              abort ();
        }
    }

    (void)serial_init(devStr);

    (void)getchar();

    retval = serial_close();

    return retval;
}

void print_usage_exit(void)
{
    puts("usage: ic7300jig [-d|--device[=]/dev/ttyUSBx] [-s|--changing_scale]");
    puts("  default device is /dev/ttyUSB0.");
    exit(1);
}
