#ifndef _PROGRAMMER_H
#define _PROGRAMMER_H

#define CODEPLUG_SIZE 262709

#define RADIO_NAME_LEN 16


/* Minimal representation of the MD380 codeplug. */
struct codeplug {
    uint8_t padding1[8805];
    uint16_t splash_line1[10];
    uint16_t splash_line2[10];
    uint8_t padding2[28];
    uint8_t radio_id[3];
    uint8_t padding3[41];
    uint16_t radio_name[16];
} __attribute__((packed));


/* Zone Record */
#define CP_ZONE_OFFSET  0x14C05
#define CP_ZONE_MAX     250

#define CP_ZONE_DEFINED(Z) ((Z).zone_name[0] != 0)
#define CP_ZONE_PTR(CP) (struct zone *)(&((uint8_t *)CP)[CP_ZONE_OFFSET]);

struct zone {
    uint16_t zone_name[16];
    uint16_t channel_members[16];
} __attribute__((packed));


/* Channel Record */
#define CP_CHANNEL_OFFSET   0x1F025
#define CP_CHANNEL_MAX      1000

#define CP_CHANNEL_DEFINED(C) ((C).freq_rx != 0xFFFFFFFF)
#define CP_CHANNEL_PTR(CP) (struct channel *)(&((uint8_t *)CP)[CP_CHANNEL_OFFSET]);

struct channel {
    uint16_t channel_config_1;
    uint16_t channel_config_2;
    uint8_t  channel_config_3;
    uint8_t  _padding1;
    uint16_t contact_name; //0=none
    uint8_t  tot; //[7:2]
    uint8_t  tot_rekey_delay;
    uint8_t  emergency_sys; //[7:2]
    uint8_t  scan_list;
    uint8_t  group_list;
    uint8_t  _padding2;
    uint8_t  decode18;
    uint8_t  _padding3;
    uint32_t freq_rx;
    uint32_t freq_tx;
    uint16_t tone_decode;
    uint16_t tone_encode;
    uint8_t  _padding4;
    uint8_t  rx_signal_sys;
    //uint8_t  tx_signal_sys;
    uint16_t _padding5;
    uint16_t channel_name[16];
} __attribute__((packed));


/* Digital Contact */
#define CP_DIGCONTACT_OFFSET    0x61A5
#define CP_DIGCONTACT_MAX       1000

#define CP_DIGCONTACT_DEFINED(DC) ((DC).call_name[0] != 0x00)
#define CP_DIGCONTACT_PTR(CP) (struct digcontact *)(&((uint8_t *)CP)[CP_DIGCONTACT_OFFSET]);

#define CP_DIGCONTACT_PRIVATE 0x80
#define CP_DIGCONTACT_GROUP   0x40
#define CP_DIGCONTACT_RXTONE  0x04

struct digcontact {
    uint8_t call_id[3];
    uint8_t call_config;
    uint16_t call_name[16];
};

#endif
