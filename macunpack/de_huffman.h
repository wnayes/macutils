#include <stdint.h>

void set_huffman(int endian);
void read_tree();
void de_huffman(uint32_t obytes);
void de_huffman_end(unsigned int term);