# icom7300_jig

Just a little program to emulate enough of my ICOM 7300 to facilitate folding
icom7300_remote_scope into flrig on Sourceforge. This program gives valid
responses to a number of requests from icom7300_remote_scope or flrig. It does
not keep track of state, e.g. if flrig changes which VFO is in use this program
does not keep track of that.

It uses 115200 baud N81. The way I would use it would be to plug two USB to
serial converters into my PC arranged as null modem. Then, run this program on
/dev/ttyUSB0 and have flrig configured to use /dev/ttyUSB1.

null modem:
    Tx---Rx
    Rx---Tx
   Gnd---Gnd

The command/response lookup is in ic7300.cxx. Reference IC-7300_ENG_CD_0.pdf,
section "CONTROL COMMAND".

Used for testing changes to flrig, particularily in rigs/ICbase.cxx.
Want to test the following. Timeouts tested for correct values too.
* Working messages
* Working messages that hesitate for almost a timeout period
* OK timeout that fails
* OK timeout that recovers
* Message timeout that fails
* Message timeout that recovers

0x1C 0x01 0x?? is intentionally not answered so that by clicking "Tune" on
flrig we can cause an OK timeout.

"PTT" on or off is intentionally answered with 0xFA which is NG (No Good).

The following optional ERROR INJECTION OPTIONS can be given to cause timeouts of various kinds on the other end:
* -p|--p_drop_byte   [=]PROB probability that each particular byte will be dropped.
* -m|--p_drop_message[=]PROB probability that message dropping will be triggered.
* -n|--n_drop_message[=]N    number of messages that will be dropped once triggered by -m.
* -h|--p_hesitate    [=]PROB probablilty that message hesitation will be triggered.
* -t|--t_hesitate    [=]T    number of milleseconds to hesitate when triggered by -h.
* -k|--n_hesitate    [=]N    number of messages that will be hesitated once triggered by -h.
PROB must be between 0.0 and 1.0 inclusive. 0.0 is the default.
T is an integer. 1000 is used if option is not given.
N is an integer. 1 is used if option is not given.

Communicate between this program and flrig on the same pc with this virtual
serial port program:
https://askubuntu.com/questions/588800/setup-virtual-serial-ports-linux-null-modem-emulator-using-tty0tty
https://github.com/freemed/tty0tty
