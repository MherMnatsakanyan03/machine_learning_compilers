#include "InstGen.h"
#include <iostream>

using gpr_t = MiniJit::Asm::gpr_t;
using simd_fp_t = MiniJit::Asm::simd_fp_t;
using arr_spec_t = MiniJit::Asm::arr_spec_t;
using namespace MiniJit;

int main()
{
	uint32_t l_ins = 0;
	std::string l_str;

	// CBNZ
	std::cout << "cbnz w0, #0" << std::endl;
	l_ins = Asm::Base::cbnz(gpr_t::w0,
							0x0);
	l_str = Asm::to_string_hex(l_ins);
	std::cout << " " << l_str << std::endl;
	l_str = Asm::to_string_bin(l_ins);
	std::cout << " " << l_str << std::endl;

	// CBNZ
	std::cout << "cbnz w5, #-100" << std::endl;
	l_ins = Asm::Base::cbnz(gpr_t::w5,
							-25);
	l_str = Asm::to_string_hex(l_ins);
	std::cout << " " << l_str << std::endl;
	l_str = Asm::to_string_bin(l_ins);
	std::cout << " " << l_str << std::endl;

	// FMLA (vector)
	std::cout << "fmla v16.2s, v29.2s, v2.2s" << std::endl;
	l_ins = Asm::SIMD::fmla_dp(simd_fp_t::v16,
							   simd_fp_t::v29,
							   simd_fp_t::v2,
							   arr_spec_t::s2);
	l_str = Asm::to_string_hex(l_ins);
	std::cout << " " << l_str << std::endl;
	l_str = Asm::to_string_bin(l_ins);
	std::cout << " " << l_str << std::endl;

	// FMLA (vector)
	std::cout << "fmla v5.2d, v3.2d, v22.2d" << std::endl;
	l_ins = Asm::SIMD::fmla_dp(simd_fp_t::v5,
							   simd_fp_t::v3,
							   simd_fp_t::v22,
							   arr_spec_t::d2);
	l_str = Asm::to_string_hex(l_ins);
	std::cout << " " << l_str << std::endl;
	l_str = Asm::to_string_bin(l_ins);
	std::cout << " " << l_str << std::endl;

	// FMLA (vector)
	std::cout << "fmla v9.4s, v31.4s, v1.4s" << std::endl;
	l_ins = Asm::SIMD::fmla_dp(simd_fp_t::v9,
							   simd_fp_t::v31,
							   simd_fp_t::v1,
							   arr_spec_t::s4);
	l_str = Asm::to_string_hex(l_ins);
	std::cout << " " << l_str << std::endl;
	l_str = Asm::to_string_bin(l_ins);
	std::cout << " " << l_str << std::endl;

	return EXIT_SUCCESS;
}