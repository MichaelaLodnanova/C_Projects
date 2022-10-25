#ifndef CAPTURE_H
#define CAPTURE_H

#include "pcap.h"

struct dataFlow {
    int counter;
    uint32_t start_sec;
    uint32_t start_usec;
    uint32_t end_sec;
    uint32_t end_usec;
    uint8_t source[4];
    uint8_t dest[4];
};

struct linkedDataFlow{
    struct dataFlow *dataNode;
    struct linkedDataFlow *next;
    struct linkedDataFlow *previous;
};

struct linkedPackets
{
    struct packet_t *packet;
    struct linkedPackets *previous;
    struct linkedPackets *next;


};

struct capture_t {

    // header
    struct pcap_header_t *header;
    // data transfer
    size_t dataTransfered;

    // linked list of packets
    struct linkedPackets *headPacket;
};


/**
 * @brief Loads entire capture from a pcap file
 *
 * @param capture
 * @param filename
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int load_capture(struct capture_t *capture, const char *filename);

/**
 * @brief Destroys capture and releases all resources allocated during
 * load_capture call
 *
 * @param capture
 */
void destroy_capture(struct capture_t *capture);

/**
 * @brief Get the pcap file header
 *
 * @param capture
 *
 * @return pointer to the header stored in capture
 */
const struct pcap_header_t* get_header(
    const struct capture_t *const capture
);

/**
 * @brief Get the packet on a given index
 *
 * @param capture
 * @param index
 *
 * @return pointer to the index-th packed stored in capture
 *         NULL in case of invalid index
 */
struct packet_t* get_packet(
    const struct capture_t *const capture,
    size_t index
);

/**
 * @brief Calculates the number of packets stored in the capture
 *
 * @param capture
 *
 * @return number of packets
 */
size_t packet_count(const struct capture_t *const capture);

/**
 * @brief Calculates the total amount of data contained in the capture.
 * The amount is calculated as a sum of orig_len of every packet contained
 * in the capture.
 *
 * @param capture
 *
 * @return amount of data contained in capture in bytes
 */
size_t data_transfered(const struct capture_t *const capture);


/**
 * @brief Filters packets with given protocol
 *
 * @param original source capture
 * @param filtered pointer to store new capture to
 * @param protocol
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int filter_protocol(
    const struct capture_t *const original,
    struct capture_t *filtered,
    uint8_t protocol
);

/**
 * @brief Filters packets with at least given size
 *
 * @param original source capture
 * @param filtered pointer to store new capture to
 * @param size
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int filter_larger_than(
    const struct capture_t *const original,
    struct capture_t *filtered,
    uint32_t size
);

/**
 * @brief Filters packets with given source and destination addresses
 *
 * @param original source capture
 * @param filtered pointer to store new capture to
 * @param source_ip
 * @param destination_ip
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int filter_from_to(
    const struct capture_t *const original,
    struct capture_t *filtered,
    uint8_t source_ip[4],
    uint8_t destination_ip[4]
);

/**
 * @brief Filters packets with source address conforming to given mask
 *        192.168.1.1/24
 *        \_________/ \/
 *      mask_address   mask_length
 *
 * @param original source capture
 * @param filtered pointer to store new capture to
 * @param mask_address ip address to derive mask from
 * @param mask_length number of bits from the address to use as mask
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int filter_from_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t mask_address[4],
        uint8_t mask_length
);

/**
 * @brief Filters packets with destination address conforming to given mask
 *        192.168.1.1/24
 *        \_________/ \/
 *      mask_address   mask_length
 *
 * @param original source capture
 * @param filtered pointer to store new capture to
 * @param mask_address ip address to derive mask from
 * @param mask_length number of bits from the address to use as mask
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int filter_to_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t mask_address[4],
        uint8_t mask_length
);

/**
 * @brief Print stats about every flow in the form:
 *        <ip_from> -> <ip_to> : <packet_count>
 *        Order of flows follows their first occurence in the capture
 *
 * @param capture
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int print_flow_stats(const struct capture_t *const capture);

/**
 * @brief Prints the longest flow in the form:
 * <ip_from> -> <ip_to> : <start_sec>:<micro/nano> - <end_sec>:<micro/nano>
 *
 * @param capture
 *
 * @return 0 in case of success, non-zero return code otherwise
 */
int print_longest_flow(const struct capture_t *const capture);


#endif // CAPTURE_H
