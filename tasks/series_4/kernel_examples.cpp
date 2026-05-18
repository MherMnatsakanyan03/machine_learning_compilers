#include "Kernel.h"
#include "Asm.h"
#include <iostream>

using namespace MiniJit;

/**
 * @brief Kernel that returns 5.
 */
void example_0()
{
    std::cout << "example_0" << std::endl;

    MiniJit::Kernel l_kernel;
    l_kernel.add_instr(0xd28000a0); // mov x0, #5
    l_kernel.add_instr(0xd65f03c0); // ret
    l_kernel.write("example_0.bin");
    l_kernel.set_kernel();

    int64_t (*l_func)() = nullptr;
    l_func = (int64_t (*)())l_kernel.get_kernel();
    std::cout << "  result: " << l_func() << std::endl;
}

/**
 * @brief Kernel that uses the given immediate
 *        to set the return value.
 * @param imm16 16-bit immediate value.
 */
void example_1(int16_t imm16)
{
    std::cout << "example_1" << std::endl;

    MiniJit::Kernel l_kernel;
    uint32_t l_ins = 0xd2800000;
    l_ins |= imm16 << 5;
    l_kernel.add_instr(l_ins);      // mov x0, #imm16
    l_kernel.add_instr(0xd65f03c0); // ret
    l_kernel.write("example_1.bin");
    l_kernel.set_kernel();

    int64_t (*l_func)() = nullptr;
    l_func = (int64_t (*)())l_kernel.get_kernel();
    std::cout << "  result: " << l_func() << std::endl;
}

/**
 * @brief Kernel that adds 5 to the passed value.
 */
void example_2()
{
    std::cout << "example_2" << std::endl;

    MiniJit::Kernel l_kernel;
    l_kernel.add_instr(0x91001400); // add x0, x0, #5
    l_kernel.add_instr(0xd65f03c0); // ret
    l_kernel.write("example_2.bin");
    l_kernel.set_kernel();

    int64_t (*l_func)(int64_t) = nullptr;
    l_func = (int64_t (*)(int64_t))l_kernel.get_kernel();
    std::cout << "  result: " << l_func(7) << std::endl;
}

/**
 * @brief Kernel that contains a loop.
 */
void example_3()
{
    std::cout << "example_3" << std::endl;

    MiniJit::Kernel l_kernel;
    l_kernel.add_instr(0xd2804000); // mov x0, #512
    l_kernel.add_instr(0xd2800001); // mov x1, #0
    l_kernel.add_instr(0xd1000400); // sub x0, x0, #1
    l_kernel.add_instr(0x91000821); // add x1, x1, #2
    l_kernel.add_instr(0xb5ffffc0); // cbnz x0, #-8
    l_kernel.add_instr(0xaa0103e0); // mov x0, x1
    l_kernel.add_instr(0xd65f03c0); // ret
    l_kernel.write("example_3.bin");
    l_kernel.set_kernel();

    int64_t (*l_func)() = nullptr;
    l_func = (int64_t (*)())l_kernel.get_kernel();
    std::cout << "  result: " << l_func() << std::endl;
}

void example_4()
{
    std::cout << "example_4 (Matrix Transpose Kernel)" << std::endl;
    using namespace MiniJit::Asm;

    MiniJit::Kernel l_kernel;

    // --- 1. PROLOGUE (Check if c <= 0) ---
    l_kernel.add_instr(Base::cmp(w4, 0)); // cmp w4, #0

    MiniJit::Kernel::Label forward_branch_idx = l_kernel.get_label();
    l_kernel.add_instr(0); // Dummy placeholder for 'ble .Lend'

    // --- 2. LOOP START ---
    MiniJit::Kernel::Label loop_start = l_kernel.get_label();

    l_kernel.add_instrs({// Loads (Q-registers mapped to v registers, scaled natively by our ldr logic)
                         SIMD::ldr(v0, x0, 0),
                         SIMD::ldr(v1, x0, 16),
                         SIMD::ldr(v2, x0, 32),
                         SIMD::ldr(v3, x0, 48),
                         SIMD::ldr(v4, x0, 64),
                         SIMD::ldr(v5, x0, 80),
                         SIMD::ldr(v6, x0, 96),
                         SIMD::ldr(v7, x0, 112),

                         // First Zip Pass (32-bit floats -> .4s)
                         SIMD::zip1(v16, v0, v1, arr_spec_t::s4),
                         SIMD::zip1(v17, v2, v3, arr_spec_t::s4),
                         SIMD::zip1(v18, v4, v5, arr_spec_t::s4),
                         SIMD::zip1(v19, v6, v7, arr_spec_t::s4),

                         // Second Zip Pass (64-bit blocks -> .2d)
                         SIMD::zip1(v20, v16, v17, arr_spec_t::d2),
                         SIMD::zip1(v21, v18, v19, arr_spec_t::d2),
                         SIMD::zip2(v22, v16, v17, arr_spec_t::d2),
                         SIMD::zip2(v23, v18, v19, arr_spec_t::d2),

                         // Third Zip Pass (32-bit floats -> .4s)
                         SIMD::zip2(v16, v0, v1, arr_spec_t::s4),
                         SIMD::zip2(v17, v2, v3, arr_spec_t::s4),
                         SIMD::zip2(v18, v4, v5, arr_spec_t::s4),
                         SIMD::zip2(v19, v6, v7, arr_spec_t::s4),

                         // Fourth Zip Pass (64-bit blocks -> .2d)
                         SIMD::zip1(v24, v16, v17, arr_spec_t::d2),
                         SIMD::zip1(v25, v18, v19, arr_spec_t::d2),
                         SIMD::zip2(v26, v16, v17, arr_spec_t::d2),
                         SIMD::zip2(v27, v18, v19, arr_spec_t::d2),

                         // Stores
                         SIMD::str(v20, x1, 0),
                         SIMD::str(v21, x1, 16),
                         SIMD::str(v22, x1, 32),
                         SIMD::str(v23, x1, 48),
                         SIMD::str(v24, x1, 64),
                         SIMD::str(v25, x1, 80),
                         SIMD::str(v26, x1, 96),
                         SIMD::str(v27, x1, 112),

                         // Advance pointers and decrement counter
                         Base::add(x0, x0, 128),
                         Base::add(x1, x1, 128),
                         Base::sub(w4, w4, 1)});

    // --- 3. LOOP BACKWARD BRANCH ---
    l_kernel.add_instr(Base::cbnz(w4, l_kernel.calc_offset(loop_start)));

    // --- 4. EPILOGUE (.Lend) ---
    MiniJit::Kernel::Label loop_end = l_kernel.get_label();

    // Backpatch the forward branch
    int32_t forward_offset = static_cast<int32_t>(loop_end) - static_cast<int32_t>(forward_branch_idx);
    l_kernel.patch_instruction(forward_branch_idx, Base::b_cond(forward_offset, bcond_t::le));

    l_kernel.add_instr(Base::ret()); // ret

    // Compile!
    l_kernel.write("example_4.bin");
    l_kernel.set_kernel();
    std::cout << "  Kernel compiled successfully! Size: " << l_kernel.get_size() << " bytes\n";

    // Note: We don't execute it immediately here because it expects valid
    // float arrays passed to x0 and x1, but the JIT compilation is complete!
}

int main()
{
    std::cout << "###########################" << std::endl;
    std::cout << "### welcome to mini_jit ###" << std::endl;
    std::cout << "###########################" << std::endl;

    // example_0();
    // example_1(25);
    // example_2();
    // example_3();
    example_4();

    return 0;
}