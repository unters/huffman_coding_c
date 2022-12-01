#ifndef HUFFMAN_CODING
#define HUFFMAN_CODING

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


#define DEFAULT_HEAP_SIZE   256 /* Must belong to (0, UINT64_MAX).  */
#define DEFAULT_STRING_SIZE 128 /* Must belong to (0, UINT64_MAX).  */


/* ________ "Public" functions and structures. ________ */

struct huffman_tree;

/* Build huffman tree for given c-string.  */
struct huffman_tree * 
huffman(char const *);

/* Compress given cstring using huffman. Returns compressed string which size
 * is returned by pointer size. Alphabet of characters with corresponding
 * huffman codes is return by pointer a.  */
uint8_t const *
compress_huffman(char const * str, uint64_t * size, uint8_t ** a);

/* Decompress cstring, that was previously compressed using huffman codes
 * algorithm implemented in function compress_huffman. Returns cstring. */
char *
decompress_huffman(uint8_t const * compressed_string,
    uint64_t size, uint8_t * alphabet);

uint64_t *
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

/* Huffman code functions.  */

/* Create node with given parameters.  */
static struct _huffman_tree_node *
_create_huffman_tree_node(char, uint64_t, bool);

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

/* Set bit at position pos of prefix code c to 1 if bit is true or to
 * 0 in other case.  */
static void
_append_prefix_code(uint8_t * c, uint8_t memb_size, uint8_t pos, bool bit);

static uint8_t
_get_prefix_code_length(uint8_t *, uint8_t memb_size);

static char *
_cast_prefix_code_to_cstring(uint8_t *, uint8_t);

/* The restored tree has the valid order of nodes, but has no info
 * about character frequencies. This tree can be used for decompressing
 * only.  */
static struct huffman_tree *
_restore_huffman_tree(uint8_t * codes);

static char *
_decompress_huffman_using_tree(uint8_t const *, 
    uint64_t, struct huffman_tree *);

static void
_swap(struct _huffman_tree_node *, struct _huffman_tree_node *);

#endif
