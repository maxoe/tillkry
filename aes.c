#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <omp.h>

typedef struct bitsblock {
		uint32_t a;
		uint32_t b;
		uint32_t c;
		uint32_t d;
	} bitsblock;

const unsigned char sbox[256] = {0x63 ,0x7c ,0x77 ,0x7b ,0xf2 ,0x6b ,0x6f ,0xc5 ,0x30 ,0x01 ,0x67 ,0x2b ,0xfe ,0xd7 ,0xab ,0x76,
								0xca ,0x82 ,0xc9 ,0x7d ,0xfa ,0x59 ,0x47 ,0xf0 ,0xad ,0xd4 ,0xa2 ,0xaf ,0x9c ,0xa4 ,0x72 ,0xc0,
								0xb7 ,0xfd ,0x93 ,0x26 ,0x36 ,0x3f ,0xf7 ,0xcc ,0x34 ,0xa5 ,0xe5 ,0xf1 ,0x71 ,0xd8 ,0x31 ,0x15,
								0x04 ,0xc7 ,0x23 ,0xc3 ,0x18 ,0x96 ,0x05 ,0x9a ,0x07 ,0x12 ,0x80 ,0xe2 ,0xeb ,0x27 ,0xb2 ,0x75,
								0x09 ,0x83 ,0x2c ,0x1a ,0x1b ,0x6e ,0x5a ,0xa0 ,0x52 ,0x3b ,0xd6 ,0xb3 ,0x29 ,0xe3 ,0x2f ,0x84,
								0x53 ,0xd1 ,0x00 ,0xed ,0x20 ,0xfc ,0xb1 ,0x5b ,0x6a ,0xcb ,0xbe ,0x39 ,0x4a ,0x4c ,0x58 ,0xcf,
								0xd0 ,0xef ,0xaa ,0xfb ,0x43 ,0x4d ,0x33 ,0x85 ,0x45 ,0xf9 ,0x02 ,0x7f ,0x50 ,0x3c ,0x9f ,0xa8,
								0x51 ,0xa3 ,0x40 ,0x8f ,0x92 ,0x9d ,0x38 ,0xf5 ,0xbc ,0xb6 ,0xda ,0x21 ,0x10 ,0xff ,0xf3 ,0xd2,
								0xcd ,0x0c ,0x13 ,0xec ,0x5f ,0x97 ,0x44 ,0x17 ,0xc4 ,0xa7 ,0x7e ,0x3d ,0x64 ,0x5d ,0x19 ,0x73,
								0x60 ,0x81 ,0x4f ,0xdc ,0x22 ,0x2a ,0x90 ,0x88 ,0x46 ,0xee ,0xb8 ,0x14 ,0xde ,0x5e ,0x0b ,0xdb,
								0xe0 ,0x32 ,0x3a ,0x0a ,0x49 ,0x06 ,0x24 ,0x5c ,0xc2 ,0xd3 ,0xac ,0x62 ,0x91 ,0x95 ,0xe4 ,0x79,
								0xe7 ,0xc8 ,0x37 ,0x6d ,0x8d ,0xd5 ,0x4e ,0xa9 ,0x6c ,0x56 ,0xf4 ,0xea ,0x65 ,0x7a ,0xae ,0x08,
								0xba ,0x78 ,0x25 ,0x2e ,0x1c ,0xa6 ,0xb4 ,0xc6 ,0xe8 ,0xdd ,0x74 ,0x1f ,0x4b ,0xbd ,0x8b ,0x8a,
								0x70 ,0x3e ,0xb5 ,0x66 ,0x48 ,0x03 ,0xf6 ,0x0e ,0x61 ,0x35 ,0x57 ,0xb9 ,0x86 ,0xc1 ,0x1d ,0x9e,
								0xe1 ,0xf8 ,0x98 ,0x11 ,0x69 ,0xd9 ,0x8e ,0x94 ,0x9b ,0x1e ,0x87 ,0xe9 ,0xce ,0x55 ,0x28 ,0xdf,
								0x8c ,0xa1 ,0x89 ,0x0d ,0xbf ,0xe6 ,0x42 ,0x68 ,0x41 ,0x99 ,0x2d ,0x0f ,0xb0 ,0x54 ,0xbb ,0x16};

// index 0 is dummy element
unsigned char rc[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
const size_t BLOCK_SIZE = 16;
const size_t KEY_SIZE = 16;
const size_t NUM_ROUNDS = 10;
const size_t ROW_SIZE = 4;
unsigned char *key;
unsigned char *plain;
unsigned char *cipher;
size_t plainSize;


void subBytes(unsigned char *state, size_t bytes) {

	#pragma omp parallel for
	for (size_t i = 0; i < bytes; ++i) {
		state[i] = sbox[state[i]];
	}
}

void addRoundKey(unsigned char *state, size_t stateSize, unsigned char *w, size_t last) {
	#pragma omp parallel for
	for (size_t i = 0; i < stateSize; ++i) {
		state[i] ^= w[i];
	}
}

bitsblock shiftl128 (uint32_t a, uint32_t b, uint32_t c, uint32_t d, size_t k) {
    assert (k <= 128);
    if (k >= 32) // shifting a 32-bit integer by more than 31 bits is "undefined"
    {
        a=b;
        b=c;
        c=d;
        d=0;
        shiftl128(a,b,c,d,k-32);
    }
    else
    {
        a = (a << k) | (b >> (32-k));
        b = (b << k) | (c >> (32-k));
        c = (c << k) | (d >> (32-k));
        d = (d << k);
    }

	bitsblock out = {a, b, c, d};
	return out;
}

void leftRotate(unsigned char *bits, size_t bytes, size_t n) {
	// bitsblock *block = (bitsblock *) bits;
	// bitsblock first = shiftl128(block->a, block->b, block->c, block->d, n);
	// bitsblock second = shiftl128(block->a, block->b, block->c, block->d, (bytes * 8) - n);

	// bitsblock out = {first.a | second.a, first.a | second.a, first.a | second.a, first.a | second.a};
	uint64_t *block = (uint64_t *) bits;
	*block = (*block << n) | (*block >> (64 - n));
}

void shiftRows(unsigned char *state, size_t bytes) {
	#pragma omp parallel for
	unsigned char oldState[plainSize];
	memcpy(oldState, state, plainSize);
	for (size_t i = 1; i < ROW_SIZE; ++i) {
		// leftRotate(&state[i * ROW_SIZE], bytes, i * 8);
		state[0 * 4 + i] = oldState[((0 + i) % 4) * 4  + i];
		state[1 * 4 + i] = oldState[((1 + i) % 4) * 4  + i];
		state[2 * 4 + i] = oldState[((2 + i) % 4) * 4  + i];
		state[3 * 4 + i] = oldState[((3 + i) % 4) * 4  + i];
	}
}

void galMul(unsigned char* x, size_t n) {
	if (n == 1) {
		return;
	} else if (n == 2) {
		int cond = *x >> 7;
		*x <<= 1;
		if (cond == 1) {
			*x ^= 0x1B;
		}
	} else if (n == 3) {
		unsigned char temp = *x;
		galMul(x, 2);
		*x ^= temp;
	}
}

void mixColumns(unsigned char *state, size_t bytes) {
	unsigned char oldState[plainSize];
	memcpy(oldState, state, plainSize);

	#pragma omp parallel for
	size_t mat[4][4] = {{2,3,1,1},{1,2,3,1},{1,1,2,3},{3,1,1,2}};
	for (size_t i = 0; i < ROW_SIZE; ++i) {
		for (size_t j = 0; j < ROW_SIZE; ++j) {
			unsigned char a = oldState[0 + i * 4];
			unsigned char b = oldState[1 + i * 4];
			unsigned char c = oldState[2 + i * 4];
			unsigned char d = oldState[3 + i * 4];

			galMul(&a, mat[j][0]);
			galMul(&b, mat[j][1]);
			galMul(&c, mat[j][2]);
			galMul(&d, mat[j][3]);

			state[j + i * 4] = a ^ b ^ c ^ d;
		}
		
	}
}

void rotWord(unsigned char *word) {
	unsigned char temp = *word;
	word[0] = word[1];
	word[1] = word[2];
	word[2] = word[3];
	word[3] = temp;
}

// void expandKey(unsigned char *key, unsigned char *w) {
// 	memcpy(w, key, KEY_SIZE);
	
// 	for (int i = 4 * 4; i < 4 * 44; i += 4) {
// 		unsigned char temp[4];
// 		memcpy(temp, w + i - 4, 4);

// 		if ((i / 4) % 4 == 0) {
// 			rotWord(temp);
// 			subBytes(temp, 4);
// 			temp[0] ^= rc[i / (4 * 4)];
// 		}

// 		memcpy(w + i, w + i - 4 * 4, 4);
// 		*((unsigned char*) w + i + 0) ^= temp[0];
// 		*((unsigned char*) w + i + 1) ^= temp[1];
// 		*((unsigned char*) w + i + 2) ^= temp[2];
// 		*((unsigned char*) w + i + 3) ^= temp[3];
// 	}
// }

void expandKey(unsigned char *key, unsigned char *w) {
	unsigned char temp[4];
	size_t i = 0;

	while (i < 4) {
		w[4 * i + 0] = key[4 * i + 0];
		w[4 * i + 1] = key[4 * i + 1];
		w[4 * i + 2] = key[4 * i + 2];
		w[4 * i + 3] = key[4 * i + 3];
		i = i + 1;
	}

	i = 4;

	size_t numBlocks = 1; // sorry
	while (i < numBlocks * (NUM_ROUNDS + 1) * 4) {
		temp[0] = w[4 * i - 4 + 0];
		temp[1] = w[4 * i - 4 + 1];
		temp[2] = w[4 * i - 4 + 2];
		temp[3] = w[4 * i - 4 + 3];
		
		if (i % 4 == 0) {
			rotWord(temp);
			subBytes(temp, 4);
			temp[0] ^= rc[i / 4];
		}

		w[4 * i + 0] = w[4 * i - 4 * 4 + 0] ^ temp[0];
		w[4 * i + 1] = w[4 * i - 4 * 4 + 1] ^ temp[1];
		w[4 * i + 2] = w[4 * i - 4 * 4 + 2] ^ temp[2];
		w[4 * i + 3] = w[4 * i - 4 * 4 + 3] ^ temp[3];

		i = i + 1;
	}
}

void encrypt() {
	size_t numBlocks = plainSize / BLOCK_SIZE;
	unsigned char state[plainSize];
	memcpy(state, plain, plainSize);

	unsigned char w[numBlocks * (NUM_ROUNDS + 1) * KEY_SIZE];

	// expand key here (differs from original alg)
	expandKey(key, w);

	addRoundKey(state, plainSize, &w[0], numBlocks - 1);

	for (size_t round = 1; round < NUM_ROUNDS; ++round) {
		subBytes(state, plainSize);
		shiftRows(state, plainSize);
		mixColumns(state, plainSize);
		// 4th parameter doesn't matter
		addRoundKey(state, plainSize, &w[round * numBlocks * KEY_SIZE], (round + 1) * numBlocks - 1);
	}

	subBytes(state, plainSize);
	shiftRows(state, plainSize);
	addRoundKey(state, plainSize, &w[NUM_ROUNDS * numBlocks * KEY_SIZE], (NUM_ROUNDS + 1) * numBlocks - 1);

	memcpy(cipher, state, plainSize);
}

unsigned char reverseByte(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

int main () {
	// bitsblock keyBlock= {0xF4C020A0, 0xA1F604FD, 0x343FAC6A, 0x7E6AE0F9};
	bitsblock keyBlock= {0xA020C0F4, 0xFD04F6A1, 0x6AAc3F34, 0xF9E06A7E};
	// bitsblock plainBlock = {0xF295B931, 0x8B994434, 0xD93D98A4, 0xE449AFD8};
	bitsblock plainBlock = {0x31B995F2, 0x3444998B, 0xA4983DD9, 0xD8AF49E4};
	bitsblock cipherBlock = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
	key = (unsigned char *) &keyBlock;
	plain = (unsigned char *) &plainBlock;
	cipher = (unsigned char *) &cipherBlock;

	plainSize = 16;

	encrypt();

	// printf("%08" PRIx32, cipherBlock.a);
	// printf("%08" PRIx32, cipherBlock.b);
	// printf("%08" PRIx32, cipherBlock.c);
	// printf("%08" PRIx32 "\n", cipherBlock.d);
	for (size_t i = 0; i < plainSize; ++i) {
		printf("%02x", cipher[i]);
	}

	printf("\n");

	return 0;
}