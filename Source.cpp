#include <iostream>

#include "regal/fixed.hpp"

int main()
{
	using namespace regal;

	double d = 12.3214389;

	fixed<std::int64_t, std::int64_t, 8> fp_a = d; // 56+8
	fixed<std::int16_t, int32_t, 4>      fp_b = d; // 12+4
	fixed<std::int16_t, int32_t, 8>      fp_c = d; // 8+8
	fixed<std::int8_t, int16_t, 2>       fp_d = d; // 6+2
	fixed<std::int32_t, int64_t, 8>      fp_e = d; // 24+8
	fixed<std::int64_t, std::int64_t, 32> fp_f = d; // 32+32

	fp_e = d;

	std::cout << "d: " << d << '\n';

	std::cout << "fp_a: " << fp_a << '\n';
	std::cout << "fp_b: " << fp_b << '\n';
	std::cout << "fp_c: " << fp_c << '\n';
	std::cout << "fp_d: " << fp_d << '\n';
	std::cout << "fp_e: " << fp_e << '\n';
	std::cout << "fp_f: " << fp_f << '\n';

	std::cout << 1.23456789_fp4_4   << '\n';
	std::cout << 12.3456789_fp8_8   << '\n';
	std::cout << 123.456789_fp16_16 << '\n';
	std::cout << 1234.56789_fp24_8  << '\n';

	std::cin.get();

	return 0;
}