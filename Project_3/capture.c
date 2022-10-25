#include <malloc.h>
#include <memory.h>
#include "capture.h"
#include <stdint.h>
#include "pcap.h"

#define FAILURE -1
#define SUCCESS 0

typedef struct pcap_header_t pcap_header_t;

int load_capture(struct capture_t *capture, const char *filename)
{
    struct pcap_context context[1];

    capture->dataTransfered = 0;

    // load error
    if (init_context(context, filename) != PCAP_SUCCESS){
        return FAILURE;
    }

    capture->header = malloc(sizeof(struct pcap_header_t));

    // load error
    if (capture->header == NULL){
        free(capture->header);
        destroy_context(context);
        return FAILURE;
    }

    // load header
    if (load_header(context, capture->header) != PCAP_SUCCESS) {
        free(capture->header);
        destroy_context(context);
        return FAILURE;
    }

    // memory alloc for the first packet
    capture->headPacket = malloc(sizeof(struct linkedPackets));

    // alloc error
    if (capture->headPacket == NULL){
        free(capture->header);
        destroy_context(context);
        return FAILURE;
    }

    struct linkedPackets *currentPacket = capture->headPacket;
    capture->headPacket->next = NULL;
    capture->headPacket->previous = NULL;
    capture->headPacket->packet = NULL;

    int loadingStatus = PCAP_SUCCESS;

    while (loadingStatus == PCAP_SUCCESS){

        currentPacket->packet = malloc(sizeof(struct packet_t));

        if (currentPacket->packet == NULL) {
            free(currentPacket);
            destroy_capture(capture);
            destroy_context(context);
            return FAILURE;
        }

        loadingStatus = load_packet(context, currentPacket->packet);

        if (loadingStatus == PCAP_SUCCESS) {
            capture->dataTransfered += currentPacket->packet->packet_header->orig_len;
            currentPacket->next = malloc(sizeof(struct linkedPackets));
            if (currentPacket->next == NULL) {
                destroy_capture(capture);
                destroy_context(context);
                return FAILURE;
            }

            currentPacket->next->previous = currentPacket;
            currentPacket = currentPacket->next;

        }
        else if (loadingStatus == PCAP_FILE_END) {
            //last packet has not been loaded - end of file
            if (currentPacket->previous != NULL){
                currentPacket->previous->next = NULL;
            }
            else {
                capture->headPacket = NULL;
            }

            free(currentPacket->packet);
            free(currentPacket);

            currentPacket = NULL;

        }

    }

    destroy_context(context);

    if (loadingStatus == PCAP_LOAD_ERROR) {
        destroy_capture(capture);
        return FAILURE;
    }

    return SUCCESS;
}

void destroy_capture(struct capture_t *capture)
{
    struct linkedPackets *currentPacket = capture->headPacket;

    while (currentPacket != NULL){
        struct linkedPackets *nextPacket = currentPacket->next;
        destroy_packet(currentPacket->packet);
        free(currentPacket->packet);
        free(currentPacket);
        currentPacket = nextPacket;
    }
    free(capture->header);
}

const struct pcap_header_t *get_header(const struct capture_t *const capture)
{
    if (capture->header != NULL){
        return capture->header;
    }
    return NULL;
}

struct packet_t *get_packet(
        const struct capture_t *const capture,
        size_t index)
{
    size_t currentIndex = 0;
    struct linkedPackets *currentPacket = capture->headPacket;

    if (currentPacket == NULL){
        return NULL;
    }

    while (currentIndex != index){
        if (currentPacket->next != NULL) {
            currentPacket = currentPacket->next;
            currentIndex++;
        } else {
            return NULL;
        }
    }
    return currentPacket->packet;
}

size_t packet_count(const struct capture_t *const capture)
{
    struct linkedPackets *current = capture->headPacket;
    size_t counter = 0;
    if (current != NULL){
        counter++;
        while (current->next != NULL){
            current = current->next;
            counter++;
        }
    }
    return counter;
}

size_t data_transfered(const struct capture_t *const capture)
{
    size_t result = capture->dataTransfered;

    return result;
}

void copy_header(struct pcap_header_t *dest, struct pcap_header_t *source){
    dest->magic_number = source->magic_number;
    dest->network = source->network;
    dest->sigfigs = source->sigfigs;
    dest->thiszone = source->thiszone;
    dest->snaplen = source->snaplen;
    dest->version_major = source->version_major;
    dest->version_minor = source->version_minor;
}

int compareIp(uint8_t ip1[4], uint8_t ip2[4])
{
    for (uint8_t i = 0; i < 4; ++i) {
        if (ip1[i] != ip2[i]){
            return FAILURE;
        }
    }
    return SUCCESS;
}

int filter_protocol(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t protocol)
{
    if (original == NULL || filtered == NULL){
        return FAILURE;
    }

    filtered->dataTransfered = 0;

    //1.malloc na header
    filtered->header = malloc(sizeof(struct pcap_header_t));
    if (filtered->header == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }
    //2. memory copy
    copy_header(filtered->header, original->header);
//____________________________________________________________________________________________________________________
    struct linkedPackets *currentPacket = original->headPacket;
    filtered->headPacket = malloc(sizeof(struct linkedPackets));
    struct linkedPackets *filteredCurrent = filtered->headPacket;


    while (currentPacket != NULL) {
        uint8_t currentProtocol = currentPacket->packet->ip_header->protocol;
        if (currentProtocol != protocol){
            currentPacket = currentPacket->next;
        }
        else
        {
            filteredCurrent->packet = malloc(sizeof(struct packet_t));

            if (filteredCurrent->packet == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }

            if (copy_packet(currentPacket->packet, filteredCurrent->packet) != PCAP_SUCCESS) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filtered->dataTransfered += currentPacket->packet->packet_header->orig_len;


            filteredCurrent->next = malloc(sizeof(struct linkedPackets));

            if (filteredCurrent->next == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filteredCurrent->next->previous = filteredCurrent;
            filteredCurrent = filteredCurrent->next;
            currentPacket = currentPacket->next;
        }
    }
    if (filteredCurrent->previous != NULL) {
        filteredCurrent = filteredCurrent->previous;
        free(filteredCurrent->next);
        filteredCurrent->next = NULL;
    }
    else {
        free(filteredCurrent);
        filtered->headPacket = NULL;
    }
    return SUCCESS;
}

int filter_larger_than(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint32_t size)
{
    if (original == NULL || filtered == NULL){
        return FAILURE;
    }

    filtered->dataTransfered = 0;

    //1.malloc na header
    filtered->header = malloc(sizeof(struct pcap_header_t));
    if (filtered->header == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }
    //2. memory copy
    copy_header(filtered->header, original->header);
//____________________________________________________________________________________________________________________
    struct linkedPackets *currentPacket = original->headPacket;
    filtered->headPacket = malloc(sizeof(struct linkedPackets));

    if (filtered->headPacket == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }

    struct linkedPackets *filteredCurrent = filtered->headPacket;


    while (currentPacket != NULL) {
        uint32_t currentSize = currentPacket->packet->packet_header->orig_len;
        if (currentSize < size){
            currentPacket = currentPacket->next;
        }
        else
        {
            filteredCurrent->packet = malloc(sizeof(struct packet_t));

            if (filteredCurrent->packet == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }

            if (copy_packet(currentPacket->packet, filteredCurrent->packet) != PCAP_SUCCESS) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filtered->dataTransfered += currentPacket->packet->packet_header->orig_len;

            filteredCurrent->next = malloc(sizeof(struct linkedPackets));

            if (filteredCurrent->next == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filteredCurrent->next->previous = filteredCurrent;
            filteredCurrent = filteredCurrent->next;
            currentPacket = currentPacket->next;
        }
    }
    if (filteredCurrent->previous != NULL) {
        filteredCurrent = filteredCurrent->previous;
        free(filteredCurrent->next);
        filteredCurrent->next = NULL;
    }
    else {
        free(filteredCurrent);
        filtered->headPacket = NULL;
    }
    return SUCCESS;
}


int filter_from_to(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t source_ip[4],
        uint8_t destination_ip[4])
{
    if (original == NULL || filtered == NULL){
        return FAILURE;
    }
    if (source_ip == NULL || destination_ip == NULL){
        return FAILURE;
    }

    filtered->dataTransfered = 0;

    //1.malloc na header
    filtered->header = malloc(sizeof(struct pcap_header_t));
    if (filtered->header == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }
    //2. memory copy
    copy_header(filtered->header, original->header);

    //__________________________________________________________________________________________//

    struct linkedPackets *currentPacket = original->headPacket;
    filtered->headPacket = malloc(sizeof(struct linkedPackets));
    struct linkedPackets *filteredCurrent = filtered->headPacket;

    while (currentPacket != NULL){
        if (compareIp(currentPacket->packet->ip_header->src_addr, source_ip) != SUCCESS ||
            compareIp(currentPacket->packet->ip_header->dst_addr, destination_ip) != SUCCESS){
            currentPacket = currentPacket->next;
        }
        else {

            filteredCurrent->packet = malloc(sizeof(struct packet_t));

            if (filteredCurrent->packet == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }

            if (copy_packet(currentPacket->packet, filteredCurrent->packet) != PCAP_SUCCESS) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filtered->dataTransfered += currentPacket->packet->packet_header->orig_len;

            filteredCurrent->next = malloc(sizeof(struct linkedPackets));

            if (filteredCurrent->next == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filteredCurrent->next->previous = filteredCurrent;
            filteredCurrent = filteredCurrent->next;
            currentPacket = currentPacket->next;
        }
    }
    if (filteredCurrent->previous != NULL) {
        filteredCurrent = filteredCurrent->previous;
        free(filteredCurrent->next);
        filteredCurrent->next = NULL;
    }
    else {
        free(filteredCurrent);
        filtered->headPacket = NULL;
    }
    return SUCCESS;
}

uint32_t parse_ipv4(uint8_t ipaddr[4])
{
    return (ipaddr[0]<<24) + (ipaddr[1] << 16) + (ipaddr[2] << 8) + ipaddr[3];
}

int filter_from_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length)
{
    if (original == NULL || filtered == NULL || mask_length > 32){
        return FAILURE;
    }

    filtered->dataTransfered = 0;

    //1.malloc na header
    filtered->header = malloc(sizeof(struct pcap_header_t));
    if (filtered->header == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }
    //2. memory copy
    copy_header(filtered->header, original->header);
//____________________________________________________________________________________________________________________
    struct linkedPackets *currentPacket = original->headPacket;
    filtered->headPacket = malloc(sizeof(struct linkedPackets));
    if (filtered->headPacket == NULL){
        destroy_capture(filtered);
        return FAILURE;
    }

    struct linkedPackets *filteredCurrent = filtered->headPacket;

    uint32_t ipNet = parse_ipv4(network_prefix); // network ip subnet mask
    uint32_t netmask = (0xFFFFFFFF << (32 - mask_length)) & 0xFFFFFFFF; // mask prefix
    while (currentPacket != NULL) {
        uint32_t ipSrc = parse_ipv4(currentPacket->packet->ip_header->src_addr); // value to check
        if (((ipSrc & netmask) != (ipNet & netmask)) && (mask_length !=0)) {
            currentPacket = currentPacket->next;
        } else {
            filteredCurrent->packet = malloc(sizeof(struct packet_t));

            if (filteredCurrent->packet == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }

            if (copy_packet(currentPacket->packet, filteredCurrent->packet) != PCAP_SUCCESS) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filtered->dataTransfered += currentPacket->packet->packet_header->orig_len;

            filteredCurrent->next = malloc(sizeof(struct linkedPackets));

            if (filteredCurrent->next == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filteredCurrent->next->previous = filteredCurrent;
            filteredCurrent = filteredCurrent->next;
            currentPacket = currentPacket->next;
        }
    }
    if (filteredCurrent->previous != NULL) {
        filteredCurrent = filteredCurrent->previous;
        free(filteredCurrent->next);
        filteredCurrent->next = NULL;
    }
    else {
        free(filteredCurrent);
        filtered->headPacket = NULL;
    }
    return SUCCESS;
}

int filter_to_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length)
{
    if (original == NULL || filtered == NULL || mask_length > 32){
        return FAILURE;
    }

    filtered->dataTransfered = 0;

    //1.malloc na header
    filtered->header = malloc(sizeof(struct pcap_header_t));
    if (filtered->header == NULL){
        destroy_capture(filtered);
        free(filtered);
        return FAILURE;
    }
    //2. memory copy
    copy_header(filtered->header, original->header);
//____________________________________________________________________________________________________________________
    struct linkedPackets *currentPacket = original->headPacket;
    filtered->headPacket = malloc(sizeof(struct linkedPackets));
    if (filtered->headPacket == NULL){
        destroy_capture(filtered);
        return FAILURE;
    }
    struct linkedPackets *filteredCurrent = filtered->headPacket;
    filteredCurrent->next = NULL;
    filteredCurrent->previous = NULL;

    uint32_t ipNet = parse_ipv4(network_prefix); // network ip subnet mask
    uint32_t netmask = (0xFFFFFFFF << (32 - mask_length)) & 0xFFFFFFFF; // mask prefix
    while (currentPacket != NULL) {
        uint32_t ipDest = parse_ipv4(currentPacket->packet->ip_header->dst_addr); // value to check
        if (((ipDest & netmask) != (ipNet & netmask)) && (mask_length !=0)) {
            currentPacket = currentPacket->next;
        } else {
            filteredCurrent->packet = malloc(sizeof(struct packet_t));

            if (filteredCurrent->packet == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }

            if (copy_packet(currentPacket->packet, filteredCurrent->packet) != PCAP_SUCCESS) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filtered->dataTransfered += currentPacket->packet->packet_header->orig_len;

            filteredCurrent->next = malloc(sizeof(struct linkedPackets));

            if (filteredCurrent->next == NULL) {
                destroy_capture(filtered);
                return FAILURE;
            }
            filteredCurrent->next->previous = filteredCurrent;
            filteredCurrent = filteredCurrent->next;
            currentPacket = currentPacket->next;
        }
    }
    if (filteredCurrent->previous != NULL) {
        filteredCurrent = filteredCurrent->previous;
        free(filteredCurrent->next);
        filteredCurrent->next = NULL;
    }
    else {
        free(filteredCurrent);
        filtered->headPacket = NULL;
    }
    return SUCCESS;
}

void copyIp(uint8_t src[4], uint8_t dst[4]){
    for (uint8_t i = 0; i < 4; ++i) {
        dst[i] = src[i];
    }
}


void destroy_data_flow(struct linkedDataFlow *structure)
{
    if (structure == NULL){
        free(structure);
    }
    else if (structure->previous == NULL){
        free(structure->dataNode);
        free(structure);
    }
    else {
        struct linkedDataFlow *currentFlow = structure;

        while (currentFlow->next != NULL){
            currentFlow = currentFlow->next;
        }
        while (currentFlow->previous != NULL) {
            struct linkedDataFlow *prevFlow = currentFlow->previous;
            free(currentFlow->dataNode);
            free(currentFlow);
            currentFlow = prevFlow;
        }
        free(currentFlow->dataNode);
        free(currentFlow);
    }
}

int load_data_flow(struct linkedPackets *currentPacket, struct linkedDataFlow *currentFlow){
    struct linkedDataFlow *headFlow = currentFlow;

    // fill the first dataFlow with data from headPacket
    currentFlow->dataNode = malloc(sizeof(struct dataFlow));
    if (currentFlow->dataNode == NULL){
        return FAILURE;
    }

    currentFlow->dataNode->counter = 0;

    if (currentPacket != NULL) {
        currentFlow->dataNode->start_sec = currentPacket->packet->packet_header->ts_sec;
        currentFlow->dataNode->start_usec = currentPacket->packet->packet_header->ts_usec;
        currentFlow->dataNode->end_sec = currentPacket->packet->packet_header->ts_sec;
        currentFlow->dataNode->end_usec = currentPacket->packet->packet_header->ts_usec;
        copyIp(currentPacket->packet->ip_header->src_addr, currentFlow->dataNode->source);
        copyIp(currentPacket->packet->ip_header->dst_addr, currentFlow->dataNode->dest);

        if (currentPacket->next == NULL){ //if we have only one packet in a flow
            return SUCCESS;
        }
        currentPacket = currentPacket->next;

        currentFlow->dataNode->counter++;
    }

    while (currentPacket != NULL){
        while (currentFlow != NULL) {
            int control = compareIp(currentPacket->packet->ip_header->src_addr, currentFlow->dataNode->source);
            int control2 = compareIp(currentPacket->packet->ip_header->dst_addr, currentFlow->dataNode->dest);
            if (control == SUCCESS && control2 == SUCCESS) {
                currentFlow->dataNode->counter++;
                currentFlow->dataNode->end_sec = currentPacket->packet->packet_header->ts_sec;
                currentFlow->dataNode->end_usec = currentPacket->packet->packet_header->ts_usec;
                break;
            }
            else{
                if (currentFlow->next == NULL){

                    currentFlow->next = malloc(sizeof(struct linkedDataFlow));

                    if (currentFlow->next == NULL){
                        return FAILURE;
                    }
                    currentFlow->next->previous = currentFlow;
                    currentFlow = currentFlow->next;
                    currentFlow->next = NULL;

                    currentFlow->dataNode = malloc(sizeof(struct dataFlow));
                    if (currentFlow->dataNode == NULL){
                        return FAILURE;
                    }

                    currentFlow->dataNode->counter = 1;
                    currentFlow->dataNode->start_sec = currentPacket->packet->packet_header->ts_sec;
                    currentFlow->dataNode->start_usec = currentPacket->packet->packet_header->ts_usec;
                    currentFlow->dataNode->end_sec = currentPacket->packet->packet_header->ts_usec;
                    currentFlow->dataNode->end_usec = currentPacket->packet->packet_header->ts_usec;
                    copyIp(currentPacket->packet->ip_header->src_addr, currentFlow->dataNode->source);
                    copyIp(currentPacket->packet->ip_header->dst_addr, currentFlow->dataNode->dest);
                    break;
                }

                currentFlow = currentFlow->next;
            }
        }
        currentFlow = headFlow;
        currentPacket = currentPacket->next;
    }

    currentFlow = headFlow;
    return SUCCESS;
}

int print_flow_stats(const struct capture_t *const capture)
{
    if (capture->headPacket == NULL){
        return SUCCESS;
    }
    // current struct pre capture headPacket
    struct linkedPackets *currentPacket = capture->headPacket;

    // alloc memory for the first linked data flow
    struct linkedDataFlow *currentFlow = malloc(sizeof(struct linkedDataFlow));

    //check allocation
    if (currentFlow == NULL){
        fprintf(stderr, "malloc failure");
        return FAILURE;
    }
    currentFlow->next = NULL;
    currentFlow->previous = NULL;

    int loadStatus = load_data_flow(currentPacket, currentFlow);
    if (loadStatus != SUCCESS){
        destroy_data_flow(currentFlow);
        fprintf(stderr, "load data flow failure");
        return FAILURE;
    }

    //_____________________PRINTING DATA FLOW_______________________________________________________

    while(currentFlow->next != NULL){
        uint8_t *src = currentFlow->dataNode->source;
        uint8_t *dest = currentFlow->dataNode->dest;
        int num = currentFlow->dataNode->counter;
        printf("%hhu.%hhu.%hhu.%hhu -> %hhu.%hhu.%hhu.%hhu : %d\n", src[0], src[1], src[2], src[3],
               dest[0], dest[1], dest[2], dest[3], num);
        currentFlow = currentFlow->next;
    }
    uint8_t *src = currentFlow->dataNode->source;
    uint8_t *dest = currentFlow->dataNode->dest;
    int num = currentFlow->dataNode->counter;
    printf("%hhu.%hhu.%hhu.%hhu -> %hhu.%hhu.%hhu.%hhu : %d\n", src[0], src[1], src[2], src[3],
           dest[0], dest[1], dest[2], dest[3], num);
    destroy_data_flow(currentFlow);
    return SUCCESS;
}

int print_longest_flow(const struct capture_t *const capture)
{
    if (capture->headPacket == NULL){
        fprintf(stderr, "empty capture");
        return FAILURE;
    }
    // current struct pre capture headPacket
    struct linkedPackets *currentPacket = capture->headPacket;

    // alloc memory for the first linked data flow
    struct linkedDataFlow *currentFlow = malloc(sizeof(struct linkedDataFlow));

    //check allocation
    if (currentFlow == NULL){
        destroy_data_flow(currentFlow);
        fprintf(stderr, "malloc failure");
        return FAILURE;
    }
    currentFlow->next = NULL;
    currentFlow->previous = NULL;

    int loadStatus = load_data_flow(currentPacket, currentFlow);
    if (loadStatus != SUCCESS){
        destroy_data_flow(currentFlow);
        fprintf(stderr, "load data flow failure");
        return FAILURE;
    }
    //____________________________________________________________________________________
    struct linkedDataFlow *pointerToMax = currentFlow;
    if(currentFlow->dataNode->counter > 1) {
        while (currentFlow->next != NULL) {
            uint32_t time = currentFlow->dataNode->end_sec - currentFlow->dataNode->start_sec;
            uint32_t uTime = currentFlow->dataNode->end_usec - currentFlow->dataNode->end_usec;
            uint32_t timeNext = currentFlow->next->dataNode->end_sec - currentFlow->dataNode->start_sec;
            uint32_t uTimeNext = currentFlow->next->dataNode->end_usec - currentFlow->dataNode->end_usec;
            if (time < timeNext || (time == timeNext && uTime < uTimeNext)) {
                pointerToMax = currentFlow->next;
            }
            currentFlow = currentFlow->next;
        }
    }
    else if(pointerToMax->dataNode->counter == 0){
        destroy_data_flow(currentFlow);
        fprintf(stderr, "no packets");
        return FAILURE;
    }
    //______________________________________PRINTING______________________________________

    // sem padne aj keď bude data node counter 1, a zároveň v celom data flow bude 1 data node
    uint8_t *src = pointerToMax->dataNode->source;
    uint8_t *dest = pointerToMax->dataNode->dest;
    uint32_t start_seconds = pointerToMax->dataNode->start_sec;
    uint32_t start_mn = pointerToMax->dataNode->start_usec;
    uint32_t end_seconds = pointerToMax->dataNode->end_sec;
    uint32_t end_mn = pointerToMax->dataNode->end_usec;

    printf("%hhu.%hhu.%hhu.%hhu -> %hhu.%hhu.%hhu.%hhu : %u:%u - %u:%u\n", src[0], src[1], src[2], src[3],
           dest[0], dest[1], dest[2], dest[3], start_seconds, start_mn, end_seconds, end_mn);

    destroy_data_flow(currentFlow);
    return SUCCESS;
}
