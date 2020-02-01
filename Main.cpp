#include <iostream>

#include "aes.h"

int main() {
	std::string path = "aes_sample.in";
	AES lab(path);
	lab.encrypt();

	return 0;
}