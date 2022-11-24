#include <stdio.h>
#include <stdint.h>

#include "./huffman.h"


void print_frequencies(struct huffman_tree *);

void print_huffman_codes(struct huffman_tree *);


int main(void) {
    char const * string = "Lorem ipsum dolor sit amet, consectetur adipiscing"
    " elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua"
    ". Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nis"
    "i ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehend"
    "erit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Exc"
    "epteur sint occaecat cupidatat non proident, sunt in culpa qui officia d"
    "eserunt mollit anim id est laborum.";

    // char const * string = "abcdefghijklmnopqrstuvwxyz";

    // char const * string = "abcd";

    struct huffman_tree * t = huffman(string);

    // print_frequencies(t);
    print_huffman_codes(t);

    return 0;
}


void print_frequencies(struct huffman_tree * t) {
    uint64_t * counts = get_char_frequencies(t);

    for (uint64_t i = 0; i < DEFAULT_HEAP_SIZE; ++i) {
        if (counts[i] == 0)
            continue;

        printf("[\'%c\':%d], ", (char)i, counts[i]);
    }

    printf("\n");
}


void print_huffman_codes(struct huffman_tree * t) {
    char * chars;
    uint8_t size, memb_size;

    char ** codes = get_huffman_codes(t, &chars, &size, &memb_size);

    for (uint16_t i = 0; i < size; ++i)
        printf("%c : %s\n", chars[i], codes[i]);
}
