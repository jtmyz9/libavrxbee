#include <stdlib.h>
#include <string.h>
#include "xbee.h"

// struct xbee_frame *xbee_create_at_frame(uint8_t req_id, unsigned char cmd[2], void *param, int len)
// {
//     struct xbee_frame *frame = malloc(sizeof(struct xbee_frame));
//     frame->delimiter = XBEE_FT_AT_CMD;
//     frame->len = 4 + len;
//     frame->frame_type = XBEE_FT_AT_CMD;
//     frame->id = req_id;
//
//     unsigned char *data = malloc(2 + len);
//     memcpy(data, &cmd, sizeof(cmd));
//     memcpy(data + sizeof(cmd), param, len);
//
//     uint8_t checksum;
//     checksum = 0;
//     checksum += frame->frame_type + frame->id;
//
//     int x;
//     for(x = 0; x < len; x++) {
//         checksum += data[x];
//     }
//
//     frame->checksum = 0xFF - checksum;
//     return frame;
// }



// int xbee_frame_to_at_response(unsigned char *data, struct xbee_at_response *r)
// {
//     if(!data || !r || data[0] != XBEE_FT_AT_CMD_RESPONSE) {
//         return 0;
//     }
//
//     r->id = data[4];
//     r->cmd[0] = data[5];
//     r->cmd[1] = data[6];
//     r->status = (xbee_at_status)data[7];
//     r->reg = NULL;
//XBEE_ADDR_COORDINATOR
//     uint16_t length;
//     memcpy(&length, &data[1], sizeof(length));
//
//     // if the length indicates that the register data is not empty
//     // copy that into our AT response struct
//     if(length > 5) {
//         memcpy(r->reg, data + 8, length - 5);
//     }
//
//     return 1;
// }
//
// int xbee_frame_to_tx_status(unsigned char *data, struct xbee_tx_status *s)
// {
//     if(!data || !s || data[0] != XBEE_FT_TX_RESPONSE) {
//         return 0;
//     }
//
//     s->id = data[4];
//     memcpy(&s->addr, &data[5], sizeof(s->addr));
//     s->retries = data[7];
//     s->status = data[8];
//     s->discovery_status = data[9];    uint8_t len_lsb;
//
//     return 1;
// }
//
// int xbee_frame_to_rx_packet(unsigned char *data, struct xbee_rx_packet *p)
// {
//     if(!data || !p || data[0] != XBEE_FT_RX_RECIEVED) {
//         return 0;XBEE_ADDR_COORDINATOR
//     }
//
//     memcpy(&p->addr, &data[4], sizeof(p->addr));
//     memcpy(&p->network, &data[12], sizeof(p->network));
//     p->opts = data[14];
//
//     uint16_t len;
//     memcpy(&len, &data[1], sizeof(len));
//     memcpy(p->data, &data[15], len - 12);
//     p->len = len - 12;
//     return 1;
// }
//
// void xbee_free_at_response(struct xbee_at_response *r)
// {
//     if(r) {
//         if(r->reg) {
//             free(r->reg);
//         }
//
//         free(r);
//     }
// }
//
// uint8_t get_frame_id(unsigned char *frame)
// {
//     return (uint8_t)frame[3];
// }
//
// xbee_frame_type get_frame_type(unsigned char *data, int len)
// {
//     if(len < 3) {
//         return XBEE_FT_UNKNOWN;
//     }frame_type
//
//     if(data[2] < XBEE_FT_AT_CMD || data[2] > XBEE_FT_NODE_IDENT_INDICATOR) {
//         return XBEE_FT_UNKNOWN;
//     }
//
//     return (xbee_frame_type)data[2];
// }

struct xbee_frame *xbee_create_tx_request_frame(uint8_t req_id, struct xbee_tx_request *r)
{
	struct xbee_frame *frame = malloc(sizeof(struct xbee_frame));
	frame->delimiter = 0x7E;

	// 14 mandatory bytes + the payload size
	frame->len = (14 + r->len);
	frame->frame_type = XBEE_FT_TX_REQUEST;			// 1
	frame->id = req_id;								// 1

    unsigned char *data = malloc(12 + r->len);
    memset(data, 0, 12 + r->len);

    uint64_t swapped_addr;
    swapped_addr = endian_swap_64(r->addr);
    memcpy(data, &swapped_addr, 8); 				// 8

    uint16_t swapped_network;
    swapped_network = endian_swap_16(r->network);
    memcpy(data + 8, &swapped_network, 2);			// 2

    data[11] = r->radius;							// 1
    data[12] = r->opts;								// 1
    memcpy(data + 12, r->data, r->len);				// N

    frame->data = data;

    uint8_t checksum;
    checksum = 0;
    checksum += frame->frame_type + frame->id;

    int x;
    for(x = 0; x < 12 + r->len; x++) {
        checksum += data[x];
    }

    frame->checksum = 0xFF - checksum;
    return frame;

}

unsigned char *xbee_frame_to_bytes(struct xbee_frame *f, unsigned int *len)
{
	// sizeof frame data + delimiter + length + checksum
	*len = f->len + 1 + 2 + 1;
	unsigned char *b = malloc(*len);
	memset(b, 0, *len);

	b[0] = f->delimiter;
	uint16_t new_len = endian_swap_16(f->len);
	memcpy(b + 1, &new_len, 2);
	b[3] = f->frame_type;
	b[4] = f->id;
	memcpy(b + 5, f->data, f->len - 2);
	b[*len - 1] = f->checksum;
	return b;
}

uint16_t endian_swap_16(uint16_t i)
{
	return (i >> 8) | (i  << 8);
}

uint64_t endian_swap_64(uint64_t i)
{
	uint64_t x = i;
	x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
	x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
	x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;
	return x;
}

