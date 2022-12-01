#include "./huffman.h"


struct huffman_tree {
    struct _huffman_tree_node * root;
};


struct _huffman_tree_node {
    char        key;
    uint64_t    value;
    bool        is_leaf;

    struct _huffman_tree_node * left;
    struct _huffman_tree_node * right;
};


struct _heap {
    struct _huffman_tree_node ** data;
    uint64_t size, max_size;
};


struct huffman_tree *
huffman(char const * string) {
    struct _heap * h = _count_char_frequencies(string);

    while(h->size > 1) {
        struct _huffman_tree_node * l = _extract_minimum(h);
        struct _huffman_tree_node * r = _extract_minimum(h);

        struct _huffman_tree_node * s = \
            _create_huffman_tree_node('\0', l->value + r->value, false);
        s->left = l; s->right = r;

        _insert(h, s);
    }

    struct huffman_tree * t = _create_huffman_tree(_minimum(h));
    free(h->data);
    free(h);

    return t;
}


uint8_t const *
compress_huffman(char const * str, uint64_t * size, uint8_t ** a) {
    struct huffman_tree * t = huffman(str);

    uint8_t memb_size = height(t) / 8 + 1;
    uint8_t ** codes;

    _get_huffman_codes(t->root, &codes, NULL, 0, memb_size);

    uint64_t * frequencies = get_char_frequencies(t);

    uint8_t counter = 0;
    for (uint16_t i = 0; i < 256; ++i)
        if (frequencies[i] != 0) ++counter;

    /* Period - number of bytes needed to store both ascii and prefix codes
     * of one character.  */
    uint8_t     period          = memb_size + 1;
    uint32_t    alphabet_size   = 2 + counter * period;

    /* First 2 bytes are used to store number of characters in alphabet and
     * number of bytes used to store prefix code for one character.  */
    uint8_t * alphabet = calloc(alphabet_size, 1);
    alphabet[0] = counter; alphabet[1] = memb_size;

    uint8_t k = 0;
    for (uint16_t i = 0; i < 256; ++i) {
        if (frequencies[i] == 0)
            continue;

        alphabet[2 + k * period] = i;
        for (size_t j = 0; j < memb_size; ++j)
            alphabet[k * period + 3 + j] = codes[i][j];
        ++k;
    }

    *a = alphabet;

    /* At first we allocate for compressed string the same amount of memory
     * it takes to store raw string.  */
    uint8_t * compressed_string = calloc(strlen(str), 1);

    uint8_t shift = 0;
    /* Compressed size equals to number of fully used bytes (that have all
     * bits set).  */
    uint64_t raw_index = 0, compressed_size = 0;

    while(str[raw_index] != '\0') {
        uint8_t * code          = codes[str[raw_index++]];
        uint8_t * shifted_code  = calloc(memb_size + 1, 1);                        
        for (uint16_t i = 0; i < memb_size; ++i)
            *(shifted_code + i) = *(code + i);

        for (uint8_t i = memb_size; i > 0; --i) {
            for (uint8_t j = 0; j < shift; ++j) {
                shifted_code[i] >>= 1;

                if (shifted_code[i - 1] & (1 << j))
                    shifted_code[i] ^= 128;
            }
        }
        
        shifted_code[0] >>= shift;

        for (uint16_t i = 0; i <= memb_size; ++i)
            compressed_string[compressed_size++] ^= shifted_code[i];

        while(compressed_string[compressed_size] == 0)
            --compressed_size;
        
        uint8_t length = _get_prefix_code_length(code, memb_size);

        shift = (length + shift) % 8;
        compressed_string[compressed_size] ^= 1 << (7 - shift);
        
        free(shifted_code);
    }

    if (shift != 0) ++compressed_size;

    uint8_t * compressed_string_2 = calloc(compressed_size, 1);
    for (uint64_t i = 0; i < compressed_size; ++i)
        compressed_string_2[i] = compressed_string[i];

    *size = compressed_size;

    /* Deallocating memory.  */
    free(t);

    for (uint16_t i = 0; i < 256; ++i)
        free(codes[i]);

    free(codes);
    free(frequencies);
    free(compressed_string);

    return compressed_string_2;
}


char *
decompress_huffman(uint8_t const * compressed_string,
    uint64_t size, uint8_t * alphabet)
{
    struct huffman_tree * t = _restore_huffman_tree(alphabet);
    return _decompress_huffman_using_tree(compressed_string, size, t);
}


uint64_t *
get_char_frequencies(struct huffman_tree * t) {
    uint64_t * counts = calloc(256, 8);  /* Alphabet size.  */
    _infix_traverse(t->root, counts);

    return counts;
}


char **
get_huffman_codes(struct huffman_tree * t, char ** chars, uint8_t * size, 
    uint8_t * memb_size)
{
    /* Memory used to store prefix code for a character always has free space
     * (from 1 to 8 bits) to store 1 in it as a separator between valid
     * prefix code and "junk" bits.  */
    *memb_size = height(t) / 8 + 1;
    uint8_t ** codes = NULL;

    _get_huffman_codes(t->root, &codes, NULL, 0, *memb_size);
    
    *size = 0;
    for (uint16_t i = 0; i < 256; ++i)
        if (codes[i] != NULL) ++(*size);
    
    *chars = calloc(*size, sizeof(char));
    char ** codes_strings = calloc(*size, sizeof(char *));

    uint8_t index = 0;
    for (uint16_t i = 0; i < 256; ++i) 
        if (codes[i] != NULL) {
            (*chars)[index] = (char)i;

            codes_strings[index++] = \
                _cast_prefix_code_to_cstring(codes[i], *memb_size);
        }

    return codes_strings;
}


uint8_t
height(struct huffman_tree * t) {
    return _height(t->root);
}


static void
_heapify_nondesc(struct _heap * q, uint64_t i) {
    if (q->size == 0)
        return;
        
    uint64_t smallest = i;
    uint64_t l = i * 2 + 1;
    uint64_t r = i * 2 + 2;

    if (l < q->size)
        if (q->data[l]->value < q->data[i]->value)
            smallest = l;

    if (r < q->size)
        if (q->data[r]->value < q->data[smallest]->value)
            smallest = r;
        
    if (smallest != i) {
        _swap(q->data[i], q->data[smallest]);
        _heapify_nondesc(q, smallest);
    }
}


static struct _huffman_tree_node *
_minimum(struct _heap * q) {
    return q->data[0];
}


static struct _huffman_tree_node *
_extract_minimum(struct _heap * q) {
    if (q->size < 1)
        return NULL;

    struct _huffman_tree_node * min = q->data[0];
    q->data[0] = q->data[q->size - 1];  // What if there is only one element left?
    q->size = q->size - 1;
    _heapify_nondesc(q, 0);
    return min;
}


static void
_insert(struct _heap * h, struct _huffman_tree_node * n) {
    /* If there is no more free space in heap - allocate new memory.  */
    if (h->size == h->max_size) {
        struct _huffman_tree_node ** new_data = \
            calloc(2 * h->size, sizeof(struct _huffman_tree_node *));

        for (uint64_t i = 0; i < h->size; ++i)
            new_data[i] = h->data[i];
        
        free(h->data);
        h->data = new_data;
    }

    /* Proceed insertion.  */
    h->size += 1;
    h->data[h->size - 1] = n;

    uint64_t i = h->size - 1;
    uint64_t parent_index = (i - 1) / 2;

    while (i > 0 && h->data[parent_index]->value > h->data[i]->value)
    {
        _swap(h->data[parent_index], h->data[i]);
        i = parent_index;
        parent_index = (i - 1) / 2;
    }
}


static struct _heap *
_initialize_heap() {
    struct _heap * h = calloc(1, sizeof(struct _heap));
    h->data = calloc(DEFAULT_HEAP_SIZE, 
        sizeof(struct _huffman_tree_node *));
    h->max_size = DEFAULT_HEAP_SIZE; h->size = 0;

    return h;
}


static struct _huffman_tree_node *
_create_huffman_tree_node(char key, uint64_t value, bool is_leaf) {
    struct _huffman_tree_node * n = \
        calloc(1, sizeof(struct _huffman_tree_node));

    n->key = key;
    n->value = value;
    n->is_leaf = is_leaf;
    n->left = NULL; n->right = NULL;

    return n;
}


struct _heap *
_count_char_frequencies(char const * string) {
    uint64_t * counts = calloc(256, 8);
    for (uint16_t j = 0; j < 256; ++j)
        counts[j] = 0;

    uint64_t i = 0;
    while (string[i] != '\0')
        counts[string[i++]] += 1;

    struct _heap * h = _initialize_heap();

    for (uint16_t j = 0; j < 256; ++j) {
        if (counts[j] == 0)
            continue;

        struct _huffman_tree_node * n = \
            _create_huffman_tree_node((char)j, counts[j], true);

        _insert(h, n);
    }

    return h;
}


struct huffman_tree *
_create_huffman_tree(struct _huffman_tree_node * n) {
    struct huffman_tree * t = calloc(1, sizeof(struct huffman_tree));
    t->root = n;
    return t;
}


static void
_infix_traverse(struct _huffman_tree_node * n, uint64_t * counts) {
    if (n == NULL)
        return;

    if (n->left == NULL && n->right == NULL) {
        counts[n->key] = n->value;   // Changed from size_t to int.
        return;
    }

    // printf("[\'%c\':%d]", n->key, n->value);
    _infix_traverse(n->left, counts);
    _infix_traverse(n->right, counts);    
}


static uint8_t
_height(struct _huffman_tree_node * n) {
    if (n->left == NULL && n->right == NULL)
        return 0;

    return _height(n->left) > _height(n->right)
        ? 1 + _height(n->left)
        : 1 + _height(n->right);
}


static void
_get_huffman_codes(struct _huffman_tree_node * n, uint8_t *** codes,
    uint8_t * path, uint8_t depth, uint8_t memb_size)
{    
    if (depth == 0) {
        *codes  = calloc(256, sizeof(uint8_t *));   // *codes = [NULL, NULL, NULL, ...]
        path    = calloc(memb_size, 1);
    }

    if (n->left == NULL && n->right == NULL) {
        uint8_t * code = _copy_prefix_code(path, memb_size);
        free(path);
        (*codes)[n->key] = code;

        return;
    }

    uint8_t * path_l = _copy_prefix_code(path, memb_size);
    uint8_t * path_r = _copy_prefix_code(path, memb_size);

    _append_prefix_code(path_l, memb_size, depth, 0);
    _append_prefix_code(path_r, memb_size, depth, 1);

    _get_huffman_codes(n->left, codes, path_l, ++depth, memb_size);
    _get_huffman_codes(n->right, codes, path_r, depth, memb_size);

    free(path);
}


static uint8_t *
_copy_prefix_code(uint8_t * code, uint8_t memb_size) {
    uint8_t * p = calloc(memb_size, 1);
    for (uint8_t i = 0; i < memb_size; ++i)
        *(p + i) = *(code + i);

    return p;
}


static void
_append_prefix_code(uint8_t * c, uint8_t memb_size, uint8_t pos, bool bit) {
    /* First of all bit at pos must be set to 0 to perform bitwise or
     * correctly (probably, bitwise or is not the right choice).  */
    if (bit)
        c[pos / 8] |= (1 << (7 - pos % 8));

    /* If bit at pos is 1 but must be set to zero, bitwise or
     * can not be used.  */
    else if (c[pos / 8] & (1 << (7 - pos % 8)))
        c[pos / 8] ^= (1 << (7 - pos % 8));

    /* Set separating 1. */
    c[++pos / 8] |= (1 << (7 - pos % 8));
}


static uint8_t
_get_prefix_code_length(uint8_t * c, uint8_t memb_size) {
    for (uint8_t i = memb_size - 1; i >= 0; --i) {
        if (c[i] == 0)
            continue;

        for (uint8_t j = 0; j < 8; ++j) /* Could change j < 8 to j < 7, because there is at leaste one 1 bit.  */
            if (c[i] & (1 << j))    /* (1 << j) in O(n2) - need separate variable?  */
                return i * 8 + (7 - j);
    }
}


static char *
_cast_prefix_code_to_cstring(uint8_t * prefix_code, uint8_t memb_size) {
    uint8_t length = _get_prefix_code_length(prefix_code, memb_size);
    char * char_code = calloc(length + 1, sizeof(char));
    
    /* There is such a string, that its huffman tree height is 255.  */
    for (uint16_t j = 0; j < length; ++j) {
        uint8_t a = prefix_code[j / 8];

        if (a & (1 << (7 - j % 8)))
            char_code[j] = '1';
                
        else
            char_code[j] = '0';
        }
        
    char_code[length] = '\0';

    return char_code;
}


static struct huffman_tree *
_restore_huffman_tree(uint8_t * codes) {
    uint8_t     counter     = codes[0];
    uint8_t     memb_size   = codes[1];
    uint16_t    period      = memb_size + 1;

    struct _huffman_tree_node * root = \
        _create_huffman_tree_node('\0', 0, false);
    struct _huffman_tree_node * node;

    char        character;
    uint32_t    code_index;

    for (uint8_t i = 0; i < counter; ++i) {
        node        = root;
        character   = codes[2 + period * i];
        code_index  = 3 + i * period;

        uint8_t length = \
            _get_prefix_code_length(codes + code_index, memb_size);

        uint8_t mask, byte;

        for (uint16_t bit = 0; bit < length - 1; ++bit) {
            byte    = bit / 8;
            mask    = bit % 8;

            if (codes[code_index + byte] & (1 << (7 - mask))) {
                if (node->right == NULL)
                    node->right = _create_huffman_tree_node('\0', 0, false);

                node = node->right;
            }

            else {
                if (node->left == NULL)
                    node->left = _create_huffman_tree_node('\0', 0, false);

                node = node->left;
            }
        }

        if (codes[code_index + (length - 1) / 8]
                & (1 << (7 - (length - 1) % 8)))
            node->right = _create_huffman_tree_node(character, 0, true);

        else
            node->left = _create_huffman_tree_node(character, 0, true);
    }

    return _create_huffman_tree(root);
}


static char *
_decompress_huffman_using_tree(uint8_t const * compressed_string,
    uint64_t size, struct huffman_tree * t)
{
    char * string = calloc(DEFAULT_STRING_SIZE, 1);
    /* Using max function is needed to prevent the situation when
     * DEFAULT_STRING_SIZE equals to 0 by some reason.  */
    uint64_t max_string_size    = DEFAULT_STRING_SIZE;
    uint64_t string_index       = 0;

    uint8_t mask = 128;  /* 0b10000000.  */
    uint64_t byte_index = 0;
    struct _huffman_tree_node * node = t->root;

    while (string_index < size) {
        if (mask == 0) {
            mask = 128;
            ++byte_index;
        }

        if (compressed_string[byte_index] & mask)
            node = node->right;

        else
            node = node->left;

        if (node->is_leaf) {
            string[string_index++] = node->key;
            node = t->root;

            if (string_index == max_string_size) {
                max_string_size = string_index * 2;
                char * new_string = calloc(max_string_size, 1);
                for (uint64_t i = 0; i < string_index; ++i)
                    new_string[i] = string[i];
                
                free(string);
                string = new_string;
            }
        }

        mask >>= 1;
    }

    string[string_index++] = '\0';

    char * decompressed_string = calloc(string_index, 1);
    for (uint64_t i = 0; i < string_index; ++i)
        decompressed_string[i] = string[i];
                
    free(string);

    return decompressed_string;
}


static void
_swap(struct _huffman_tree_node * p, struct _huffman_tree_node * q) {
    if (p == q)
        return;

    struct _huffman_tree_node t;
    t.key = p->key; t.value = p->value;
    t.left = p->left; t.right = p->right;
    t.is_leaf = p->is_leaf;

    p->key = q->key; p->value = q->value;
    p->left = q->left; p->right = q->right;
    p->is_leaf = q->is_leaf;

    q->key = t.key; q->value = t.value;
    q->left = t.left; q->right = t.right;
    q->is_leaf = t.is_leaf;
}
