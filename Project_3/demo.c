#include <stdio.h>
#include <stdlib.h>
#include "capture.h"

#define TEST_FILE "test.pcap"

void demo1()
{
    struct pcap_context context[1];
    if (init_context(context, TEST_FILE) != PCAP_SUCCESS) {
        return;
    }

    struct pcap_header_t *pcap_header = malloc(sizeof(struct pcap_header_t));
    if (pcap_header == NULL) {
        destroy_context(context);
        return;
    }

    if (load_header(context, pcap_header) != PCAP_SUCCESS) {
        free(pcap_header);
        destroy_context(context);
        return;
    }

    struct packet_t *packet1 = malloc(sizeof(struct packet_t));

    if (packet1 == NULL) {
        free(pcap_header);
        destroy_context(context);
        return;
    }

    if (load_packet(context, packet1) != PCAP_SUCCESS) {
        free(pcap_header);
        destroy_context(context);
        return;
    }

    struct packet_t *packet2 = malloc(sizeof(struct packet_t));

    if (packet2 == NULL) {
        free(packet1);
        free(pcap_header);
        destroy_context(context);
        return;
    }

    if (load_packet(context, packet2) != PCAP_SUCCESS) {
        free(packet1);
        free(pcap_header);
        destroy_context(context);
        return;
    }

    destroy_context(context);

    printf("packet 1:\n");
    print_packet_info(packet1);

    printf("\npacket 2:\n");
    print_packet_info(packet2);

    destroy_packet(packet1);
    destroy_packet(packet2);

    free(packet1);
    free(packet2);
    free(pcap_header);
}

void demo2()
{
    struct capture_t *capture = malloc(sizeof(struct capture_t));
    load_capture(capture, TEST_FILE);

    for (size_t current_packet = 0; current_packet < packet_count(capture); current_packet++) {
        struct packet_t *packet = get_packet(capture, current_packet);
        print_packet_info(packet);
    }

    printf("Magic number: 0x%x\n", get_header(capture)->magic_number);
    printf("Total number of bytes transferred in this capture: %zu.\n", data_transfered(capture));
    destroy_capture(capture);
    free(capture);
}

int main()
{
    demo1();

    demo2();
}
