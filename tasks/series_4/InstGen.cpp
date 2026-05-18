#include "InstGen.h"
#include <sstream>
#include <iomanip>
#include <bitset>

namespace MiniJit::Asm
{
	std::string to_string_hex(uint32_t inst)
	{
		std::stringstream l_ss;
		l_ss << "0x" << std::hex
			 << std::setfill('0')
			 << std::setw(8)
			 << inst;

		return l_ss.str();
	}

	std::string to_string_bin(uint32_t inst)
	{
		std::string l_res = "0b";
		l_res += std::bitset<32>(inst).to_string();

		return l_res;
	}

	/* ====================================================== Base ====================================================== */

	namespace Base
	{
		uint32_t cbnz(gpr_t reg,
					  int32_t imm19)
		{
			uint32_t l_ins = 0b0'011010'1'0000000000000000000'00000;

			// set register id
			uint32_t l_reg_id = reg & 0b00011111;
			l_ins |= l_reg_id;

			// set size of the register
			uint32_t l_reg_size = reg & 0b00100000;
			l_ins |= l_reg_size << (32 - 6);

			// set immediate
			uint32_t l_imm = imm19 & 0b01111111111111111111;
			l_ins |= l_imm << 5;

			return l_ins;
		}

		uint32_t ret(gpr_t reg = gpr_t::x30)
		{
			// ARM Doc: 1101011001011111000000 | Rn[9:5] | 00000
			uint32_t l_ins = 0b1101011001011111000000'00000'00000;

			uint32_t l_rn = reg & 0x1F; // Extract 5-bit register ID
			l_ins |= (l_rn << 5);		// Shift into the Rn field

			return l_ins;
		}

		uint32_t ldr(gpr_t reg_dest, gpr_t reg_base, int64_t imm)
		{
			// Base Unsigned Offset Load encoding:
			// size[31:30] | 111 0 01 | opc[23:22]=01 | imm12[21:10] | Rn[9:5] | Rt[4:0]
			uint32_t l_ins = 0b00'111'0'01'01'000000000000'00000'00000;

			// Extract raw register IDs (masking out size/type flags from the enum)
			uint32_t l_rt = reg_dest & 0b00011111;
			uint32_t l_rn = reg_base & 0b00011111;

			// Determine access size and required scaling factor
			bool l_is_64bit = (reg_dest & 0b00100000) != 0;
			uint32_t l_scale = l_is_64bit ? 3 : 2; // 2^3 = 8 bytes (x), 2^2 = 4 bytes (w)
			uint64_t l_byte_size = 1ULL << l_scale;

			// Validate alignment and immediate boundaries
			if (__builtin_expect(imm < 0, 0))
			{
				throw std::invalid_argument("LDR (unsigned): immediate offset cannot be negative.");
			}
			if (__builtin_expect((imm & (l_byte_size - 1)) != 0, 0))
			{
				throw std::invalid_argument("LDR: immediate offset is not correctly aligned to the register size.");
			}

			uint64_t l_scaled_imm = imm >> l_scale;
			if (__builtin_expect(l_scaled_imm > 0b0000111111111111, 0))
			{ // Max 12-bit unsigned value (4095)
				throw std::invalid_argument("LDR: immediate offset exceeds maximum encoded range.");
			}

			// Encode the bitfields
			if (l_is_64bit)
			{
				l_ins |= (3U << 30); // Set size to 0b11 for 64-bit access
			}
			else
			{
				l_ins |= (2U << 30); // Set size to 0b10 for 32-bit access
			}

			l_ins |= (l_scaled_imm << 10);
			l_ins |= (l_rn << 5);
			l_ins |= l_rt;

			return l_ins;
		}

		uint32_t str(gpr_t reg_src, gpr_t reg_base, int64_t imm)
		{
			// Base Unsigned Offset Store encoding:
			// size[31:30] | 111 0 01 | opc[23:22]=00 | imm12[21:10] | Rn[9:5] | Rt[4:0]
			uint32_t l_ins = 0b00'111'0'01'00'000000000000'00000'00000;

			uint32_t l_rt = reg_src & 0b00011111;
			uint32_t l_rn = reg_base & 0b00011111;

			bool l_is_64bit = (reg_src & 0b00100000) != 0;
			uint32_t l_scale = l_is_64bit ? 3 : 2;
			uint64_t l_byte_size = 1ULL << l_scale;

			if (__builtin_expect(imm < 0, 0))
			{
				throw std::invalid_argument("STR (unsigned): immediate offset cannot be negative.");
			}
			if (__builtin_expect((imm & (l_byte_size - 1)) != 0, 0))
			{
				throw std::invalid_argument("STR: immediate offset is not correctly aligned to the register size.");
			}

			uint64_t l_scaled_imm = imm >> l_scale;
			if (__builtin_expect(l_scaled_imm > 0b0000111111111111, 0))
			{
				throw std::invalid_argument("STR: immediate offset exceeds maximum encoded range.");
			}

			if (l_is_64bit)
			{
				l_ins |= (3U << 30);
			}
			else
			{
				l_ins |= (2U << 30);
			}

			l_ins |= (l_scaled_imm << 10);
			l_ins |= (l_rn << 5);
			l_ins |= l_rt;

			return l_ins;
		}
	}

	/* ====================================================== Neon ====================================================== */

	namespace SIMD
	{
		uint32_t ldr(simd_fp_t reg_dest, gpr_t reg_base, int64_t imm)
		{
			// SIMD Unsigned Offset Load (128-bit Q-register)
			// ARM Doc: size[31:30]=00 | 111 101 | opc[23:22]=11 | imm12[21:10] | Rn[9:5] | Rt[4:0]
			uint32_t l_ins = 0b00'111101'11'000000000000'00000'00000;

			uint32_t l_rt = reg_dest & 0x1F;
			uint32_t l_rn = reg_base & 0x1F;

			// Q-registers are 128-bit (16 bytes). Scale = 2^4.
			uint32_t l_scale = 4;
			uint64_t l_byte_size = 1ULL << l_scale;

			if (__builtin_expect(imm < 0, 0))
			{
				throw std::invalid_argument("SIMD LDR: immediate offset cannot be negative.");
			}
			if (__builtin_expect((imm & (l_byte_size - 1)) != 0, 0))
			{
				throw std::invalid_argument("SIMD LDR: immediate offset must be 16-byte aligned.");
			}

			uint64_t l_scaled_imm = imm >> l_scale;
			if (__builtin_expect(l_scaled_imm > 0xFFF, 0))
			{
				throw std::invalid_argument("SIMD LDR: immediate offset exceeds maximum encoded range.");
			}

			l_ins |= (l_scaled_imm << 10);
			l_ins |= (l_rn << 5);
			l_ins |= l_rt;

			return l_ins;
		}

		uint32_t str(simd_fp_t reg_src, gpr_t reg_base, int64_t imm)
		{
			// SIMD Unsigned Offset Store (128-bit Q-register)
			// ARM Doc: size[31:30]=00 | 111 101 | opc[23:22]=10 | imm12[21:10] | Rn[9:5] | Rt[4:0]
			uint32_t l_ins = 0b00'111101'10'000000000000'00000'00000;

			uint32_t l_rt = reg_src & 0x1F;
			uint32_t l_rn = reg_base & 0x1F;

			// Q-registers are 128-bit (16 bytes). Scale = 2^4.
			uint32_t l_scale = 4;
			uint64_t l_byte_size = 1ULL << l_scale;

			if (__builtin_expect(imm < 0, 0))
			{
				throw std::invalid_argument("SIMD STR: immediate offset cannot be negative.");
			}
			if (__builtin_expect((imm & (l_byte_size - 1)) != 0, 0))
			{
				throw std::invalid_argument("SIMD STR: immediate offset must be 16-byte aligned.");
			}

			uint64_t l_scaled_imm = imm >> l_scale;
			if (__builtin_expect(l_scaled_imm > 0xFFF, 0))
			{
				throw std::invalid_argument("SIMD STR: immediate offset exceeds maximum encoded range.");
			}

			l_ins |= (l_scaled_imm << 10);
			l_ins |= (l_rn << 5);
			l_ins |= l_rt;

			return l_ins;
		}

		uint32_t fmla_dp(simd_fp_t reg_dest,
						 simd_fp_t reg_src1,
						 simd_fp_t reg_src2,
						 arr_spec_t arr_spec)
		{
			uint32_t l_ins = 0b00'001110'001'00000'110011'00000'00000;

			// set destination register id
			uint32_t l_reg_id = reg_dest & 0b00011111;
			l_ins |= l_reg_id;

			// set first source register id
			l_reg_id = reg_src1 & 0b00011111;
			l_ins |= l_reg_id << 5;

			// set second source register id
			l_reg_id = reg_src2 & 0b00011111;
			l_ins |= l_reg_id << 16;

			// set arrangement specifier
			uint32_t l_arr_spec = arr_spec & 0b01000000010000000000000000000000;
			l_ins |= l_arr_spec;

			return l_ins;
		}

		uint32_t zip1(simd_fp_t reg_dest, simd_fp_t reg_src1, simd_fp_t reg_src2, arr_spec_t arr_spec)
		{
			// ZIP1 baseline (with Q=0 and size=00)
			// ARM Doc: 0 | Q[30] | 001110 | size[23:22] | 0 | Rm[20:16] | 0 | 001 | 110 | Rn[9:5] | Rd[4:0]
			uint32_t l_ins = 0b0'0'001110'00'0'00000'0'001'110'00000'00000;

			// Apply arrangement specifier mapping (Q and size)
			if (arr_spec == arr_spec_t::s2)
			{
				l_ins |= (0U << 30); // Q = 0 (64-bit vector)
				l_ins |= (2U << 22); // size = 10 (32-bit elements)
			}
			else if (arr_spec == arr_spec_t::s4)
			{
				l_ins |= (1U << 30); // Q = 1 (128-bit vector)
				l_ins |= (2U << 22); // size = 10 (32-bit elements)
			}
			else if (arr_spec == arr_spec_t::d2)
			{
				l_ins |= (1U << 30); // Q = 1 (128-bit vector)
				l_ins |= (3U << 22); // size = 11 (64-bit elements)
			}

			// Apply registers
			uint32_t l_rd = reg_dest & 0x1F;
			uint32_t l_rn = reg_src1 & 0x1F;
			uint32_t l_rm = reg_src2 & 0x1F;

			l_ins |= (l_rm << 16);
			l_ins |= (l_rn << 5);
			l_ins |= l_rd;

			return l_ins;
		}

		uint32_t zip2(simd_fp_t reg_dest, simd_fp_t reg_src1, simd_fp_t reg_src2, arr_spec_t arr_spec)
		{
			// ZIP2 baseline (with Q=0 and size=00)
			// ARM Doc: 0 | Q[30] | 001110 | size[23:22] | 0 | Rm[20:16] | 0 | 011 | 110 | Rn[9:5] | Rd[4:0]
			uint32_t l_ins = 0b0'0'001110'00'0'00000'0'011'110'00000'00000;

			// Apply arrangement specifier mapping (Q and size)
			if (arr_spec == arr_spec_t::s2)
			{
				l_ins |= (0U << 30); // Q = 0 (64-bit vector)
				l_ins |= (2U << 22); // size = 10 (32-bit elements)
			}
			else if (arr_spec == arr_spec_t::s4)
			{
				l_ins |= (1U << 30); // Q = 1 (128-bit vector)
				l_ins |= (2U << 22); // size = 10 (32-bit elements)
			}
			else if (arr_spec == arr_spec_t::d2)
			{
				l_ins |= (1U << 30); // Q = 1 (128-bit vector)
				l_ins |= (3U << 22); // size = 11 (64-bit elements)
			}

			// Apply registers
			uint32_t l_rd = reg_dest & 0x1F;
			uint32_t l_rn = reg_src1 & 0x1F;
			uint32_t l_rm = reg_src2 & 0x1F;

			l_ins |= (l_rm << 16);
			l_ins |= (l_rn << 5);
			l_ins |= l_rd;

			return l_ins;
		}
	}
}
