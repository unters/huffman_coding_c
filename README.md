# Huffman Coding (work in progress)

## Languages

[placeholder]

## Description

This repository contains implementation of huffman coding algorithm in C for educational purposes.

#### Goal

Huffman algorithm must be implemented as a cmd tool that must be capable of:

1.  compressing arbitrary files with size less or equal to 4Gb and saving the result into file with name (default or given by user) and .huf extention;

2. decompressing files with .huf extention (previosly compressed by the implementation now described).

#### Current state

Current version of algorithm is capable of compressing (hardcoded) cstrings and serializing huffman tree used for compression; and correctly decompressing the result of compression back using restored huffman tree. There is also a function to get huffman codes as cstrings (for given cstring).

## Problems

1. Current serialization mechanism is not optimal by space (see ***canonical huffman codes***);

2. To meet the requirements it is enough to use uint32_t type for counting character frequencies. However current implementation uses uint64_t, which is not only too much (2 ^ 64 = 16 millions of terabytes), but also creates some potential problems (e.g. infinite loops in cases when given cstring is of UINT64_MAX size);

3. Function that traverses huffman tree is recursive. To travere it iteratively stack is needed;

4. There are no checks for calloc returning NULL;

5. Char type is not the best choice because The Standart does not provide it with exact size;

6. Field *is_leaf* of structure *huffman_tree* is deprecated;

7. Better variable names could have been chosen.


