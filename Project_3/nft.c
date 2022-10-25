#include <stdlib.h>
#include <string.h>

#include "capture.h"

int main(int argc, char *argv[]) {

    //1. Check the correct number of arguments argc == 5
    if (argc != 5) {
        fprintf(stderr, "incorrect input");
        return EXIT_FAILURE;
    }


    //2. nacitanie argumentov
    int total_read;


    //2.b. nacitanie <from+mask>
    uint8_t from_network_prefix[4];
    uint8_t from_mask_length;
    total_read = sscanf(argv[2],
                        "%hhu.%hhu.%hhu.%hhu/%hhu",
                        &from_network_prefix[0],
                        &from_network_prefix[1],
                        &from_network_prefix[2],
                        &from_network_prefix[3],
                        &from_mask_length);
    if (total_read != 5 || from_mask_length > 32) {
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }

    //2.c. nacitanie <to+mask>
    uint8_t to_network_prefix[4];
    uint8_t to_mask_length;
    total_read = sscanf(argv[3],
                        "%hhu.%hhu.%hhu.%hhu/%hhu",
                        &to_network_prefix[0],
                        &to_network_prefix[1],
                        &to_network_prefix[2],
                        &to_network_prefix[3],
                        &to_mask_length);
    if (total_read != 5 || to_mask_length > 32) {
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }

//load capture
// check file
    FILE *file;
    if((file = fopen(argv[1],"r"))!=NULL)
    {
        // file exists
        fclose(file);
    }
    else
    {
        fprintf(stderr, "no file, add file please");
        return EXIT_FAILURE;
    }
    struct capture_t capture[1];
    if (load_capture(capture, argv[1]) != 0) {
        destroy_capture(capture);
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }
 

//3. odfiltrujeme co nie je treba


    int retval;
    struct capture_t filtered[1];
    retval = filter_to_mask(
            capture,
            filtered,
            to_network_prefix,
            to_mask_length);
    if (retval != 0) {
        destroy_capture(capture);
        destroy_capture(filtered);
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }

    struct capture_t filtered2[1];
    retval = filter_from_mask(
            filtered,
            filtered2,
            from_network_prefix,
            from_mask_length);
    if (retval != 0) {
        destroy_capture(capture);
        destroy_capture(filtered);
        destroy_capture(filtered2);
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }


    //4.a  handle flowstats
    if (strcmp(argv[4], "flowstats") == 0 ) {
        if(print_flow_stats(filtered2) != 0){
            destroy_capture(capture);
            destroy_capture(filtered);
            destroy_capture(filtered2);
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[4], "longestflow") == 0 ) {
        if(print_longest_flow(filtered2) != 0){
            destroy_capture(capture);
            destroy_capture(filtered);
            destroy_capture(filtered2);
            return EXIT_FAILURE;
        }
    }
    else {
        destroy_capture(capture);
        destroy_capture(filtered);
        destroy_capture(filtered2);
        fprintf(stderr, "program failure");
        return EXIT_FAILURE;
    }
    //4.b  handle longestflow

    destroy_capture(capture);
    destroy_capture(filtered);
    destroy_capture(filtered2);
    return EXIT_SUCCESS;
}
