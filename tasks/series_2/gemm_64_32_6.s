    .text
    .type gemm_64_32_6, %function
    .global gemm_64_32_6

gemm_64_32_6:
    // Task: implement C = C + A * B gemm for k = 64; m = 32; n = 6
    // First: load C (possibly partially) in to memory
    // Second: load A and B chunk-wise and accumulate on C

    // Check if c <= 0. If so, exit early to avoid an infinite loop.
    cmp w3, #0
    ble .Lend
    
    // PROLOGUE: Save callee-saved registers (d8-d15)
    stp d8, d9, [sp, #-64]!    // Pre-index: allocate 64 bytes and store d8, d9
    stp d10, d11, [sp, #16]    // Store d10, d11 at sp + 16
    stp d12, d13, [sp, #32]    // Store d12, d13 at sp + 32
    stp d14, d15, [sp, #48]    // Store d14, d15 at sp + 48

    // Generate a predicate for 32-bit floats (all lanes true)
    ptrue p0.s
    
    // Column 0
    ldr z0, [x2, #0, mul vl] // same as #0 here
    ldr z1, [x2, #1, mul vl] // same as #32 here
    ldr z2, [x2, #2, mul vl] // same as #64 here
    ldr z3, [x2, #3, mul vl] // ...
    // Column 1
    ldr z4, [x2, #4, mul vl]
    ldr z5, [x2, #5, mul vl]
    ldr z6, [x2, #6, mul vl]
    ldr z7, [x2, #7, mul vl]
    // Column 2
    ldr z8, [x2, #8, mul vl]
    ldr z9, [x2, #9, mul vl]
    ldr z10, [x2, #10, mul vl]
    ldr z11, [x2, #11, mul vl]
    // Column 3
    ldr z12, [x2, #12, mul vl]
    ldr z13, [x2, #13, mul vl]
    ldr z14, [x2, #14, mul vl]
    ldr z15, [x2, #15, mul vl]
    // Column 4
    ldr z16, [x2, #16, mul vl]
    ldr z17, [x2, #17, mul vl]
    ldr z18, [x2, #18, mul vl]
    ldr z19, [x2, #19, mul vl]
    // Column 5
    ldr z20, [x2, #20, mul vl]
    ldr z21, [x2, #21, mul vl]
    ldr z22, [x2, #22, mul vl]
    ldr z23, [x2, #23, mul vl]

.Lloop:
    // Load a column of A
    ldr z24, [x0, #0, mul vl]
    ldr z25, [x0, #1, mul vl]
    ldr z26, [x0, #2, mul vl]
    ldr z27, [x0, #3, mul vl]

    // Accumulate Column 0: C_col0 += A * B[0]
    ld1rw z28.s, p0/z, [x1, #0]     // Load B[0] (bytes offset 0) and broadcast to all lanes
    fmla z0.s, p0/m, z24.s, z28.s
    fmla z1.s, p0/m, z25.s, z28.s
    fmla z2.s, p0/m, z26.s, z28.s
    fmla z3.s, p0/m, z27.s, z28.s

    // Accumulate Column 1: C_col1 += A * B[1]
    ld1rw z28.s, p0/z, [x1, #4]     // Load B[1]
    fmla z4.s, p0/m, z24.s, z28.s
    fmla z5.s, p0/m, z25.s, z28.s
    fmla z6.s, p0/m, z26.s, z28.s
    fmla z7.s, p0/m, z27.s, z28.s

    // Accumulate Column 2: C_col2 += A * B[2]
    ld1rw z28.s, p0/z, [x1, #8]     // Load B[2]
    fmla z8.s, p0/m, z24.s, z28.s
    fmla z9.s, p0/m, z25.s, z28.s
    fmla z10.s, p0/m, z26.s, z28.s
    fmla z11.s, p0/m, z27.s, z28.s

    // Accumulate Column 3: C_col3 += A * B[3]
    ld1rw z28.s, p0/z, [x1, #12]    // Load B[3]
    fmla z12.s, p0/m, z24.s, z28.s
    fmla z13.s, p0/m, z25.s, z28.s
    fmla z14.s, p0/m, z26.s, z28.s
    fmla z15.s, p0/m, z27.s, z28.s

    // Accumulate Column 4: C_col4 += A * B[4]
    ld1rw z28.s, p0/z, [x1, #16]    // Load B[4]
    fmla z16.s, p0/m, z24.s, z28.s
    fmla z17.s, p0/m, z25.s, z28.s
    fmla z18.s, p0/m, z26.s, z28.s
    fmla z19.s, p0/m, z27.s, z28.s

    // Accumulate Column 5: C_col5 += A * B[5]
    ld1rw z28.s, p0/z, [x1, #20]    // Load B[5]
    fmla z20.s, p0/m, z24.s, z28.s
    fmla z21.s, p0/m, z25.s, z28.s
    fmla z22.s, p0/m, z26.s, z28.s
    fmla z23.s, p0/m, z27.s, z28.s

    // Advance the input and output pointers
    add x0, x0, #128
    add x1, x1, #24

    // Decrement the loop counter 'c'
    sub w3, w3, #1

    // Branch to .Lloop if w3 is not zero (Compare and Branch on Non-Zero)
    cbnz w3, .Lloop

.Lend:
    str z0, [x2, #0, mul vl]
    str z1, [x2, #1, mul vl]
    str z2, [x2, #2, mul vl]
    str z3, [x2, #3, mul vl]
    str z4, [x2, #4, mul vl]
    str z5, [x2, #5, mul vl]
    str z6, [x2, #6, mul vl]
    str z7, [x2, #7, mul vl]
    str z8, [x2, #8, mul vl]
    str z9, [x2, #9, mul vl]
    str z10, [x2, #10, mul vl]
    str z11, [x2, #11, mul vl]
    str z12, [x2, #12, mul vl]
    str z13, [x2, #13, mul vl]
    str z14, [x2, #14, mul vl]
    str z15, [x2, #15, mul vl]
    str z16, [x2, #16, mul vl]
    str z17, [x2, #17, mul vl]
    str z18, [x2, #18, mul vl]
    str z19, [x2, #19, mul vl]
    str z20, [x2, #20, mul vl]
    str z21, [x2, #21, mul vl]
    str z22, [x2, #22, mul vl]
    str z23, [x2, #23, mul vl]

    // EPILOGUE: Restore callee-saved registers
    ldp d14, d15, [sp, #48]
    ldp d12, d13, [sp, #32]
    ldp d10, d11, [sp, #16]
    ldp d8, d9, [sp], #64

    ret
