/* TODO:    Change all voids, representing one byte memory fields, for uint8_t. */

#ifndef HUFFMAN_CODING
#define HUFFMAN_CODING

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>


#define DEFAULT_HEAP_SIZE 256


/* ________ "Public" functions and structures. ________ */

struct huffman_tree;

/* Encoding c-string using huffman codes.  */
struct huffman_tree * 
huffman(char const *);

char const *
encode(char const *, size_t *);

size_t *
get_char_frequencies(struct huffman_tree *);

/* Gets: huffman_tree t, pointer to an array chars where characters to encode
 * will be stored, number of characters to encode size, min number of bytes,
 * memb_size needed to store any huffman code of any character in given
 * huffman tree. Returns pointer to an array of c-string huffman codes.  */
char **
get_huffman_codes(struct huffman_tree * t, char ** chars, uint8_t * size, 
    uint8_t * memb_size);

uint8_t
height(struct huffman_tree *);


/* ________ "Private"  functions and structures. ________ */

struct _huffman_tree_node;

struct _heap;

/* Heap operations.  */

static void
_heapify_nondesc(struct _heap *, uint64_t);

/* Priority queue operations.  */

static struct _huffman_tree_node *
_minimum(struct _heap *);

static struct _huffman_tree_node *
_extract_minimum(struct _heap *);

static void
_insert(struct _heap *, struct _huffman_tree_node *);

/* Create empty heap of DEFAULT_HEAP_SIZE size.  */
static struct _heap *
_initialize_heap();

/* Create node with given parameters.  */
static struct _huffman_tree_node *
_create_huffman_tree_node(char, uint64_t);

static struct _heap *
_count_char_frequencies(char const *);

static struct huffman_tree *
_create_huffman_tree(struct _huffman_tree_node *);

static void
_infix_traverse(struct _huffman_tree_node *, uint64_t *);

static uint8_t
_height(struct _huffman_tree_node *);

/* Recursively fill array of huffman codes "codes". depth stands for n'th 
 * node height (counting from top). path stores bits representing path to
 * node n in huffman tree. memb_size represents number of bytes, needed to 
 * store longest huffman code for given huffman tree.  */
static void
_get_huffman_codes(struct _huffman_tree_node * n, uint8_t *** codes,
    uint8_t * path, uint8_t depth, uint8_t memb_size);

static uint8_t *
_copy_prefix_code(uint8_t *, uint8_t);

/* Sets bit at position pos of prefix code c to 1 if bit is true or to
 * 0 in other case.  */
static void
_append_prefix_code(uint8_t * c, uint8_t memb_size, uint8_t pos, uint8_t bit);

static uint8_t
_get_prefix_code_length(uint8_t *, uint8_t memb_size);

static char *
_cast_prefix_code_to_cstring(uint8_t *, uint8_t);

static void
_swap(struct _huffman_tree_node *, struct _huffman_tree_node *);

#endif
