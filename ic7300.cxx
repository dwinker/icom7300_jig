#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "g_opt.h"
#include "ic7300.h"
#include "serial.h"
#include "scope_waveform_data.h"

const unsigned char SCOPE_CMD = 0x27;

const unsigned char SCOPE_WAVEFORM_DATA_SUBCMD        = 0x00;
const unsigned char SCOPE_OFF_ON_SUBCMD               = 0x10;
const unsigned char SCOPE_WAVEFORM_DATA_OUTPUT_SUBCMD = 0x11;
const unsigned char SCOPE_MAIN_OR_SUB_SUBCMD          = 0x12;
const unsigned char SCOPE_SINGLE_OR_DUAL_SUBCMD       = 0x13;
const unsigned char SCOPE_CENTER_OR_FIXED_SUBCMD      = 0x14;
const unsigned char SCOPE_CENTER_SPAN_SUBCMD          = 0x15;
const unsigned char SCOPE_FIXED_EDGE_SUBCMD           = 0x16;
const unsigned char SCOPE_HOLD_SUBCMD                 = 0x17;
const unsigned char SCOPE_REF_LEVEL_SUBCMD            = 0x19;
const unsigned char SCOPE_SPEED_SUBCMD                = 0x1A;
const unsigned char SCOPE_CENTER_TX_INDICATOR_SUBCMD  = 0x1B;
const unsigned char SCOPE_CENTER_MODE_SUBCMD          = 0x1C;
const unsigned char SCOPE_VBW_SUBCMD                  = 0x1D;
const unsigned char SCOPE_FIXED_EDGE_FREQS_SUBCMD     = 0x1E;

static void process_scope_cmd_from_controller(const unsigned char *buf, int length);
static void process_scope_data_subcmd_from_controller(const unsigned char *buf, int length);
static void process_other_cmd_from_controller(const unsigned char *buf, int length);

// cmd  start with FE FE  E0 94 end with FD
// resp start with FE FE  94 E0 end with FD

typedef struct cmd_resp_tag {
    unsigned char cmd[6];
    unsigned char compare_len;
    unsigned char cmd_len;
    unsigned char resp[7];
    unsigned char resp_len;
} cmd_resp_t;

enum COMMANDS {
    OPERATING_FREQ_GET,
    OPERATING_FREQ_SET,
    SELECT_VFO,
    SPLIT_GET,
    SPLIT_SET,
    ATT_SET,
    ATT_GET,
    AF_RF_SET,
    AF_LEVEL_GET,
    RF_GAIN_GET,
    SQUELCH_LEVEL_GET,
    NR_LEVEL_GET,
    RF_PWR_GET,
    MIC_GET,
    NOTCH_POS_GET,
    COMP_LEVEL_GET,
    NB_LEVEL_GET,
    S_METER_GET,
    PREAMP_GET,
    AGC_GET,
    NB_GET,
    NR_GET,
    AUTO_NOTCH_GET,
    COMP_GET,
    NOTCH_GET,
    AF_RF_2_SET,
    MEM_SET,
    FILTER_WIDTH_GET,
    REF_FREQ_GET,
    REF_FREQ_SET,
    STATUS_RX_GET,
    STATUS_RX_TX_SET,
    VFO_SET,
    VFO_GET,
};

// Note, 0x1C 0x01 0x?? is intentionally left out of this list so that by
// clicking "Tune" on flrig we can cause an OK timeout. "PTT" on or off is
// intentionally answered with NG (No Good).
static const cmd_resp_t cmd_resp_list[] = {
    {{0x03                  }, 1, 1, {0x03, 0x00, 0x60, 0x14, 0x07, 0x00}, 6},  // OPERATING_FREQ_GET
    {{0x05, 0x00            }, 2, 6, {0xFB                              }, 1},  // OPERATING_FREQ_SET
    {{0x07                  }, 1, 2, {0xFB                              }, 1},  // SELECT_VFO
    {{0x0F                  }, 1, 1, {0x0F, 0x00                        }, 2},  // SPLIT_GET
    {{0x0F                  }, 1, 2, {0xFB                              }, 1},  // SPLIT_SET
    {{0x11                  }, 1, 2, {0xFB                              }, 1},  // ATT_SET
    {{0x11                  }, 1, 1, {0x11, 0x00                        }, 2},  // ATT_GET
    {{0x14                  }, 1, 4, {0xFB                              }, 1},  // AF_RF_SET
    {{0x14, 0x01            }, 2, 2, {0x14, 0x01, 0x00, 0x20            }, 4},  // AF_LEVEL_GET
    {{0x14, 0x02            }, 2, 2, {0x14, 0x02, 0x00, 0x39            }, 4},  // RF_GAIN_GET
    {{0x14, 0x03            }, 2, 2, {0x14, 0x03, 0x00, 0x26            }, 4},  // SQUELCH_LEVEL_GET
    {{0x14, 0x06            }, 2, 2, {0x14, 0x06, 0x00, 0x08            }, 4},  // NR_LEVEL_GET
    {{0x14, 0x0A            }, 2, 2, {0x14, 0x0A, 0x00, 0x20            }, 4},  // RF_PWR_GET
    {{0x14, 0x0B            }, 2, 2, {0x14, 0x0B, 0x00, 0x00            }, 4},  // MIC_GET
    {{0x14, 0x0D            }, 2, 2, {0x14, 0x0D, 0x01, 0x00            }, 4},  // NOTCH_POS_GET
    {{0x14, 0x0E            }, 2, 2, {0x14, 0x0E, 0x00, 0x11            }, 4},  // COMP_LEVEL_GET
    {{0x14, 0x12            }, 2, 2, {0x14, 0x12, 0x01, 0x28            }, 4},  // NB_LEVEL_GET
    {{0x15, 0x02            }, 2, 2, {0x15, 0x02, 0x00, 0x00            }, 4},  // S_METER_GET
    {{0x16, 0x02            }, 2, 2, {0x16, 0x02, 0x00                  }, 3},  // PREAMP_GET
    {{0x16, 0x12            }, 2, 2, {0x16, 0x12, 0x02                  }, 3},  // AGC_GET
    {{0x16, 0x22            }, 2, 2, {0x16, 0x22, 0x00                  }, 3},  // NB_GET
    {{0x16, 0x40            }, 2, 2, {0x16, 0x40, 0x00                  }, 3},  // NR_GET
    {{0x16, 0x41            }, 2, 2, {0x16, 0x41, 0x00                  }, 3},  // AUTO_NOTCH_GET
    {{0x16, 0x44            }, 2, 2, {0x16, 0x44, 0x00                  }, 3},  // COMP_GET
    {{0x16, 0x48            }, 2, 2, {0x16, 0x48, 0x00                  }, 3},  // NOTCH_GET
    {{0x16                  }, 1, 3, {0xFB                              }, 1},  // AF_RF_2_SET
    {{0x1A                  }, 1, 3, {0xFB                              }, 1},  // MEM_SET
    {{0x1A, 0x03            }, 2, 2, {0x1A, 0x03, 0x28                  }, 3},  // FILTER_WIDTH_GET
    {{0x1A, 0x05, 0x00, 0x58}, 4, 4, {0x1A, 0x05, 0x00, 0x58, 0x00, 0x73}, 6},  // REF_FREQ_GET
    {{0x1A, 0x05            }, 2, 5, {0xFB                              }, 1},  // REF_FREQ_SET
    {{0x1C, 0x00            }, 2, 2, {0x1C, 0x00, 0x00                  }, 3},  // STATUS_RX_GET
    {{0x1C, 0x00            }, 2, 3, {0xFA                              }, 1},  // STATUS_RX_TX_SET - Answer with 0xFA which is NG (No Good)
    {{0x26, 0x00            }, 2, 5, {0xFB                              }, 1},  // VFO_SET
    {{0x26, 0x00            }, 2, 2, {0x26, 0x00, 0x00, 0x00, 0x01      }, 5},  // VFO_GET
};

int process_cmd_from_controller(const unsigned char *buf, int nread)
{
    // Spin past the 0xFEs.
    while(nread && PREAMBLE == *buf) {
        buf++;
        nread--;
    }

    // Next will be the transceiver address.
    if(nread && XCVR_ADDR == *buf) {
        buf++;
        nread--;
    } else {
        printf("Bad transceiver address: %02X\n", *buf);
        return -1;
    }

    // Next will be the controller address.
    if(nread && (CONT_ADDR == *buf || 0 == *buf)) {
        buf++;
        nread--;
    } else {
        printf("Bad controller address: %02X\n", *buf);
        return -1;
    }
    
    // Nothing to do if no more data.
    if(nread) {
        // Next byte is the command.
        switch(*buf) {
            case SCOPE_CMD:
                ++buf;
                --nread;
                process_scope_cmd_from_controller(buf, nread);
                break;
            default:
                process_other_cmd_from_controller(buf, nread);
                break;
        }
    }

    return 0;
}

static void process_scope_cmd_from_controller(const unsigned char *buf, int length)
{
    // Nothing to do if no sub command.
    if(0 == length)
        return;

    --length;
    switch(*buf++) {
        case SCOPE_WAVEFORM_DATA_SUBCMD:
            puts("SCOPE_WAVEFORM_DATA_SUBCMD unhandled");
            break;
        case SCOPE_OFF_ON_SUBCMD:
            puts("SCOPE_OFF_ON_SUBCMD unhandled");
            break;
        case SCOPE_WAVEFORM_DATA_OUTPUT_SUBCMD:
            process_scope_data_subcmd_from_controller(buf, length);
            break;
        case SCOPE_MAIN_OR_SUB_SUBCMD:
            puts("SCOPE_MAIN_OR_SUB_SUBCMD unhandled");
            break;
        case SCOPE_SINGLE_OR_DUAL_SUBCMD:
            puts("SCOPE_SINGLE_OR_DUAL_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_OR_FIXED_SUBCMD:
            puts("SCOPE_CENTER_OR_FIXED_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_SPAN_SUBCMD:
            puts("SCOPE_CENTER_SPAN_SUBCMD unhandled");
            break;
        case SCOPE_FIXED_EDGE_SUBCMD:
            puts("SCOPE_FIXED_EDGE_SUBCMD unhandled");
            break;
        case SCOPE_HOLD_SUBCMD:
            puts("SCOPE_HOLD_SUBCMD unhandled");
            break;
        case SCOPE_REF_LEVEL_SUBCMD:
            puts("SCOPE_REF_LEVEL_SUBCMD unhandled");
            break;
        case SCOPE_SPEED_SUBCMD:
            puts("SCOPE_SPEED_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_TX_INDICATOR_SUBCMD:
            puts("SCOPE_CENTER_TX_INDICATOR_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_MODE_SUBCMD:
            puts("SCOPE_CENTER_MODE_SUBCMD unhandled");
            break;
        case SCOPE_VBW_SUBCMD:
            puts("SCOPE_VBW_SUBCMD unhandled");
            break;
        case SCOPE_FIXED_EDGE_FREQS_SUBCMD:
            puts("SCOPE_FIXED_EDGE_FREQS_SUBCMD unhandled");
            break;
        default:
            printf("Unknown scope sub command: %02X\n", buf[-1]);
            break;
    }
}

static void process_scope_data_subcmd_from_controller(const unsigned char *buf, int length)
{
    bool good;

    good = (2 == length && END_MESSAGE == buf[1]);

    if(good) {
        if(0 == buf[0])
            stop_scope_waveform_thread();
        else if(1 == buf[0])
            start_scope_waveform_thread();
        else
            good = false;
    }

    if(!good) {
        printf("Bad scope data subcmd:");
        for(int n = 0; n < length; n++)
            printf(" %02X", buf[n]);
        putchar('\n');
    }
}

static void process_other_cmd_from_controller(const unsigned char *buf, int length)
{
    static int drop_message_triggered = 0;
    static int hesitate_triggered = 0;
    const cmd_resp_t *cr;
    size_t i, j;
    unsigned char xbuf[10];
    int n;

    // Nothing to do if no sub command.
    if(0 == length) {
        puts("process_other_cmd_from_controller: Can't process 0 length command.");
        return;
    }

    --length;
    if(END_MESSAGE != buf[length]) {
        puts("process_other_cmd_from_controller: Missing END_MESSAGE.");
        return;
    }

    // Arranged so we always get at least one good message between differnt
    // instances of drop_message triggered.
    if(drop_message_triggered) {
        drop_message_triggered--;
    } else if(random() <= g_opt_p_drop_message) {
        drop_message_triggered = g_opt_n_drop_message;
    }

    if(drop_message_triggered)
        return;

    // Arranged so we always get at least one good message between differnt
    // instances of hesitate triggered.
    if(hesitate_triggered) {
        hesitate_triggered--;
    } else if(random() <= g_opt_p_hesitate) {
        hesitate_triggered = g_opt_n_hesitate;
    }

    if(hesitate_triggered)
        usleep((useconds_t)g_opt_t_hesitate * 1000ul);

    for(i = 0; i < sizeof(cmd_resp_list) / sizeof(cmd_resp_list[0]); i++) {

        cr = &cmd_resp_list[i];

//        printf("i=%2d, compare_len=%d, cmd_len=%d, resp_len=%d\n",
//                (int)i, (int)cr->compare_len, (int)cr->cmd_len, (int)cr->resp_len);
//        printf("cmd: ");
//        for(j = 0; j < cr->cmd_len; j++)
//            printf(" %02X", cr->cmd[j]);
//        printf("\nresp:");
//        for(j = 0; j < cr->resp_len; j++)
//            printf(" %02X", cr->resp[j]);
//        putchar('\n');

        if(length == cr->cmd_len && 0 == memcmp(cr->cmd, buf, cr->compare_len)) {
            // Match
            n = 0;

            if(random() > g_opt_p_drop_byte) xbuf[n++] = PREAMBLE;  else printf("D %d\n", n);
            if(random() > g_opt_p_drop_byte) xbuf[n++] = PREAMBLE;  else printf("D %d\n", n); 
            if(random() > g_opt_p_drop_byte) xbuf[n++] = CONT_ADDR; else printf("D %d\n", n); 
            if(random() > g_opt_p_drop_byte) xbuf[n++] = XCVR_ADDR; else printf("D %d\n", n); 

            for(j = 0; j < cr->resp_len; j++)
                if(random() > g_opt_p_drop_byte) xbuf[n++] = cr->resp[j]; else printf("D %d\n", n); 

            if(random() > g_opt_p_drop_byte) xbuf[n++] = END_MESSAGE; else printf("D %d\n", n);

            (void)serial_send(xbuf, n);

            break;
        }
    } 
}

