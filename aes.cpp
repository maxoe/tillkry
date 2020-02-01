#include <iostream>
#include <fstream>
#include <array>
#include <bitset>

#include <omp.h>

#include "aes.h"

AES::AES(const std::string& path, AES::BlockMode mode)
	: mode(mode) {
	// std::ifstream file(key);
	// std::string hexKey((std::istreambuf_iterator<char>(file)),
    //              std::istreambuf_iterator<char>());
	
	std::ifstream input(path, std::ios::binary);
	
	std::vector<unsigned char> keyBuf(KEY_SIZE, 0);
  	input.read(reinterpret_cast<char*>(keyBuf.data()), KEY_SIZE);
	this->key = bitset_from_bytes<KEY_SIZE * 8>(keyBuf);

	std::vector<unsigned char> blockBuf(BLOCK_SIZE, 0);
  	input.read(reinterpret_cast<char*>(blockBuf.data()), BLOCK_SIZE);

	// dirty because it's just one block in the lab
	this->plain = bitset_from_bytes<BLOCK_SIZE * 8>(blockBuf);
	
	// while (input.gcount() != 0) {
	// 	bitset_from_bytes<BLOCK_SIZE>(blockBuf);
	// 	this->plain.insert(this->plain.end(), buf.begin(), buf.end());
	// 	input.read(reinterpret_cast<char*>(buf.data()), BLOCK_SIZE);
	// }
}

template<std::size_t N> std::bitset<N> AES::bitset_from_bytes(const std::vector<unsigned char>& buf)
{
    // assert(buf.size() == ((N + 7) >> 3));
    std::bitset<N> result;
    for (int j=0; j<int(N); j++)
        result[j] = ((buf[j>>3] >> (j & 7)) & 1);
    return result;
}

// AES::AES(const std::string& key, const std::string& plain, AES::BlockMode mode)
// 	: mode(mode) {
// 		this->key = fromHexAscii(key);
// 		this->plain = fromHexAscii(plain);
// }

// template<std::size_t N> std::bitset<N> AES::fromHexAscii(const std::string& string) {
// 	Bytes map{std::byte(0x0), std::byte(0x1), std::byte(0x2), std::byte(0x3), std::byte(0x4), std::byte(0x5),
// 								std::byte(0x6), std::byte(0x7), std::byte(0x8), std::byte(0x9), std::byte(0x0), std::byte(0x0), 
// 								std::byte(0x0), std::byte(0x0), std::byte(0x0), std::byte(0x0), std::byte(0x0), std::byte(0xA),
// 								std::byte(0xB), std::byte(0xC), std::byte(0xD), std::byte(0xE), std::byte(0xF)};
// 	std::size_t byteLength = string.length() / 2;
// 	std::bitmap<N> out;

// 	for (std::size_t i = 0; i < byteLength; ++i) {
// 		out[i] = (map[string[2 * i] - 0x30] << 4) | map[string[2 * i + 1] - 0x30];
// 	}

// 	return out;
// }

// std::string AES::toHexAscii(const Bytes& bytes) {
// 	std::string map = {{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}};
// 	std::string out(bytes.size() * 2, ' ');

// 	for(int i = 0 ; i < bytes.size(); ++i) {
//          out[2 * i] = map[((char) bytes[i] & 0xF0) >> 4];
// 		 out[2 * i + 1] = map[(char) bytes[i] & 0x0F];		 
// 	}
	
// 	return out;
// }

void AES::encrypt() {
	std::size_t numBlocks = this->plain.size() / BLOCK_SIZE;
	std::bitset<msgsize> state(this->plain);
	std::vector<std::bitset<KEY_SIZE * 8>> w(numBlocks * (NUM_ROUNDS + 1), std::bitset<KEY_SIZE * 8>());

	addRoundKey(state, w, 0, numBlocks - 1);

	for (std::size_t round = 1; round < NUM_ROUNDS; ++round) {
		subBytes(state);
		shiftRows(state);
		mixColumns(state);
		addRoundKey(state, w, round * numBlocks, (round + 1) * numBlocks - 1);
	}

	subBytes(state);
	shiftRows(state);
	addRoundKey(state, w, NUM_ROUNDS * numBlocks, (NUM_ROUNDS + 1) * numBlocks - 1);

	this->cipher = state;
}

// void AES::addRoundKey(const Bytes& state, const std::vector<Bytes>& w) {
// 	// addRoundKey(state, )
// }

void AES::addRoundKey(const std::bitset<msgsize>& state, const std::vector<std::bitset<KEY_SIZE * 8>>& w, std::size_t begin, std::size_t end) {
	// addRoundKey(state, std::span(w.begin() + begin, w.end() + end));
}

void AES::subBytes(std::bitset<msgsize>& state) {

	#pragma omp parallel for
	for (std::size_t i = 0; i < state.size(); ++i) {
		bool blubb = state[i];
		state[i * 8] = ()sbox[*((unsigned char*) &blubb)];
	}
}

void AES::shiftRows(std::bitset<msgsize>& state) {
	#pragma omp parallel for
	for (std::size_t i = 0; i < state.size(); ++i) {
		std::bitset<ROW_SIZE * 8> bits(0);
		// bits << state;
		leftRotate<ROW_SIZE * 8>(bits, i * 8);
	}
}

void AES::mixColumns(const std::bitset<msgsize>& state) {

}

template<std::size_t size> inline void AES::leftRotate(std::bitset<size>& bits, std::size_t n) {
	n = 129 % bits.size();
    bits = (bits << n) | (bits >> (bits.size() - n));
}