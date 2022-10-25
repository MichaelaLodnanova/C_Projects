#define main student_main
#include "../nft.c"
#undef main

#include "cut.h"

#define TEST_FILE "test.pcap"
#define MY_TEST "first_try.pcap"
#define THIRD "test_3.pcap"

TEST(flow_stats)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "0.0.0.0/0", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "74.125.19.17 -> 172.16.11.12 : 2\n"
            "216.34.181.45 -> 172.16.11.12 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}


TEST(flow_stats_with_mask)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "172.16.11.5/24", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "172.16.11.12 -> 74.125.19.17 : 3\n"
            "172.16.11.12 -> 216.34.181.45 : 2\n");
}

TEST(no_packets_in_capture)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", MY_TEST, "172.16.11.5/24", "0.0.0.0/8", "longestflow", NULL });

    CHECK(retval != 0); //no packets in capture after filter
}

TEST(longest_flow)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", TEST_FILE, "0.0.0.0/0", "0.0.0.0/0", "longestflow", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "216.34.181.45 -> 172.16.11.12 : 1278472580:873217 - 1278472581:8800\n");
}

TEST(longest_flow_bigger)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", THIRD, "0.0.0.0/0", "0.0.0.0/0", "longestflow", NULL });

    CHECK(retval == 0);
    CHECK_FILE(
            stdout,
            "10.254.0.10 -> 10.0.0.150 : 1419005464:672141 - 1419005466:559798\n");
}

TEST(flow_stats_with_mask_failure)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", THIRD, "172.16.11.5/24", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
}

TEST(flow_stats_longer)
{
    int retval = student_main(
            5,
            (char *[6]){ "./nft", THIRD, "0.0.0.0/0", "0.0.0.0/0", "flowstats", NULL });

    CHECK(retval == 0);
    CHECK_FILE(stdout,
               "10.0.0.150 -> 10.254.0.10 : 1050\n"
               "10.254.0.10 -> 10.0.0.150 : 1050\n");
}
//./nft /home/xlodnan/pb071/hw03/test_3.pcap 172.16.11.5/24 0.0.0.0/0 flowstats
