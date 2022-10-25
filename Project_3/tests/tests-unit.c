#include "cut.h"

#include "../capture.h"
#include "../pcap.h"
#define MY_TEST "first_try.pcap"
#define SECOND "my_test_2.pcap"
#define TEST_FILE "test.pcap"
#define THIRD "test_3.pcap"
#define FROM_TO_TEST "filter_from_to_test.pcap"


TEST(trying_everythin){
    struct capture_t capture[1];
    int retval = load_capture(capture, SECOND);
    ASSERT(retval == 0);
    size_t data = data_transfered(capture);
    ASSERT(data != 0);
    ASSERT(packet_count(capture) == 28U);
    CHECK(get_packet(capture, 13)->ip_header->protocol == 6U);
    struct capture_t filtered[1];
    retval = filter_protocol(capture, filtered, 6U);
    ASSERT(retval == 0);
    ASSERT(packet_count(filtered) == 2U);
    destroy_capture(capture);
    destroy_capture(filtered);
}


TEST(load_capture_basic)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    size_t nwm = data_transfered(capture);
    ASSERT(nwm != 0);

    CHECK(packet_count(capture) == 10U);

    // Check the length of the first packet
    CHECK(get_packet(capture, 0)->packet_header->orig_len == 93U);

    // Check the length of the last packet
    CHECK(get_packet(capture, 9)->packet_header->orig_len == 1514U);

    destroy_capture(capture);
}

TEST(filter_from_to_basic)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_from_to(
            capture,
            filtered,
            (uint8_t[4]){ 74U, 125U, 19U, 17U },
            (uint8_t[4]){ 172U, 16U, 11U, 12U });
    ASSERT(retval == 0);

    CHECK(packet_count(filtered) == 2U);

    // Check lengths of both packets
    CHECK(get_packet(filtered, 0)->packet_header->orig_len == 66U);
    CHECK(get_packet(filtered, 1)->packet_header->orig_len == 66U);

    destroy_capture(capture);
    destroy_capture(filtered);
}

TEST(filter_to_mask_basic)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_to_mask(
            capture,
            filtered,
            (uint8_t[4]){ 216U, 34U, 0U, 0U },
            16U);
    ASSERT(retval == 0);

    CHECK(packet_count(filtered) == 2U);

    // Check lengths of both packets
    CHECK(get_packet(filtered, 0)->ip_header->dst_addr[3] == 45U);
    CHECK(get_packet(filtered, 1)->ip_header->dst_addr[3] == 45U);

    destroy_capture(capture);
    destroy_capture(filtered);
}

TEST(filter_from_mask_basic)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_from_mask(
            capture,
            filtered,
            (uint8_t[4]){ 216U, 34U, 0U, 0U },
            16U);
    ASSERT(retval == 0);

    CHECK(packet_count(filtered) == 3U);

    // Check lengths of both packets
    CHECK(get_packet(filtered, 0)->ip_header->src_addr[3] == 45U);
    CHECK(get_packet(filtered, 1)->ip_header->src_addr[3] == 45U);
    CHECK(get_packet(filtered, 2)->ip_header->src_addr[3] == 45U);

    destroy_capture(capture);
    destroy_capture(filtered);
}


TEST(print_flow_stats)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    print_flow_stats(capture);

    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "74.125.19.17 -> 172.16.11.12 : 2\n"
            "216.34.181.45 -> 172.16.11.12 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");

    destroy_capture(capture);
}


TEST(print_longets_flow_basic)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, TEST_FILE);
    ASSERT(retval == 0);

    print_longest_flow(capture);

    CHECK_FILE(
            stdout,
            "216.34.181.45 -> 172.16.11.12 : 1278472580:873217 - 1278472581:8800\n");

    destroy_capture(capture);
}

TEST(filter_protocol_my)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, MY_TEST);
    ASSERT(retval == 0);
    size_t counter = packet_count(capture);
    ASSERT(counter == 454U);
    struct capture_t filtered[1];

    retval = filter_protocol(capture, filtered, 6); //tcp protocol
    ASSERT(retval == 0);


    destroy_capture(capture);
    destroy_capture(filtered);
}

TEST(count)
{
    struct capture_t capture[1];
    int retval = load_capture(capture, THIRD);
    ASSERT(retval == 0);
    CHECK(packet_count(capture) == 2100U);

    destroy_capture(capture);
}

TEST(filter_protocol_longestFlow)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, THIRD);
    ASSERT(retval == 0);

    size_t counter = packet_count(capture);
    ASSERT(counter == 2100U);

    struct capture_t filtered[1];

    retval = filter_protocol(capture, filtered, 6); //tcp protocol
    ASSERT(filtered->headPacket == NULL);

    destroy_capture(capture);
    destroy_capture(filtered);
}
TEST(filter_protocol_my_test_2)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, SECOND);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_protocol(capture, filtered, 6);
    ASSERT(retval == 0);


    destroy_capture(capture);
    destroy_capture(filtered);
}

TEST(filter_larger_than_size_100)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, SECOND);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_larger_than(capture, filtered, 100);
    ASSERT(retval == 0);
    ASSERT(packet_count(filtered) == 14U);


    destroy_capture(capture);
    destroy_capture(filtered);
}
TEST(filter_from_to_ipv4)
{
    struct capture_t capture[1];

    int retval = load_capture(capture, FROM_TO_TEST);
    ASSERT(retval == 0);

    struct capture_t filtered[1];

    retval = filter_from_to(capture, filtered,
                            (uint8_t[4]){ 192U,168U,0U ,104U},
            (uint8_t[4]){239U, 255U, 255U, 250U});
    ASSERT(retval == 0);
    ASSERT(packet_count(filtered) == 12U);


    destroy_capture(capture);
    destroy_capture(filtered);
}


