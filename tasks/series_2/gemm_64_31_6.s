    .text
    .type gemm_64_31_6, %function
    .global gemm_64_31_6

gemm_64_31_6:
    // Task: implement C = C + A * B gemm for k = 64; m = 31; n = 6
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
    // p1 = Exactly 7 lanes true (for the final 7 elements of the column)
    ptrue p1.s, vl7
    
    mov x5, x2 // Copy C's base pointer to x5
    
    // Column 0
    ld1w z0.s, p0/z, [x5, #0, mul vl]
    ld1w z1.s, p0/z, [x5, #1, mul vl]
    ld1w z2.s, p0/z, [x5, #2, mul vl]
    ld1w z3.s, p1/z, [x5, #3, mul vl]
    add x5, x5, #124 // jump ahead for 31 elements
    // Column 1                     
    ld1w z4.s, p0/z, [x5, #0, mul vl]
    ld1w z5.s, p0/z, [x5, #1, mul vl]
    ld1w z6.s, p0/z, [x5, #2, mul vl]
    ld1w z7.s, p1/z, [x5, #3, mul vl]
    add x5, x5, #124 // jump ahead again for 31 elements
    // Column 2                     
    ld1w z8.s, p0/z, [x5, #0, mul vl]
    ld1w z9.s, p0/z, [x5, #1, mul vl]
    ld1w z10.s, p0/z, [x5, #2, mul vl]
    ld1w z11.s, p1/z, [x5, #3, mul vl]
    add x5, x5, #124 // jump ahead again for 31 elements
    // Column 3                     
    ld1w z12.s, p0/z, [x5, #0, mul vl]
    ld1w z13.s, p0/z, [x5, #1, mul vl]
    ld1w z14.s, p0/z, [x5, #2, mul vl]
    ld1w z15.s, p1/z, [x5, #3, mul vl]
    add x5, x5, #124 // jump ahead again for 31 elements
    // Column 4                     
    ld1w z16.s, p0/z, [x5, #0, mul vl]
    ld1w z17.s, p0/z, [x5, #1, mul vl]
    ld1w z18.s, p0/z, [x5, #2, mul vl]
    ld1w z19.s, p1/z, [x5, #3, mul vl]
    add x5, x5, #124 // jump ahead again for 31 elements
    // Column 5                     
    ld1w z20.s, p0/z, [x5, #0, mul vl]
    ld1w z21.s, p0/z, [x5, #1, mul vl]
    ld1w z22.s, p0/z, [x5, #2, mul vl]
    ld1w z23.s, p1/z, [x5, #3, mul vl]

.Lloop:
    // Load a column of A
    ld1w z24.s, p0/z, [x0, #0, mul vl]
    ld1w z25.s, p0/z, [x0, #1, mul vl]
    ld1w z26.s, p0/z, [x0, #2, mul vl]
    ld1w z27.s, p1/z, [x0, #3, mul vl]

    // Accumulate Column 0: C_col0 += A * B[0]
    ld1rw z28.s, p0/z, [x1, #0]     // Load B[0] (bytes offset 0) and broadcast to all lanes
    fmla z0.s, p0/m, z24.s, z28.s
    fmla z1.s, p0/m, z25.s, z28.s
    fmla z2.s, p0/m, z26.s, z28.s
    fmla z3.s, p1/m, z27.s, z28.s

    // Accumulate Column 1: C_col1 += A * B[1]
    ld1rw z28.s, p0/z, [x1, #4]     // Load B[1]
    fmla z4.s, p0/m, z24.s, z28.s
    fmla z5.s, p0/m, z25.s, z28.s
    fmla z6.s, p0/m, z26.s, z28.s
    fmla z7.s, p1/m, z27.s, z28.s

    // Accumulate Column 2: C_col2 += A * B[2]
    ld1rw z28.s, p0/z, [x1, #8]     // Load B[2]
    fmla z8.s, p0/m, z24.s, z28.s
    fmla z9.s, p0/m, z25.s, z28.s
    fmla z10.s, p0/m, z26.s, z28.s
    fmla z11.s, p1/m, z27.s, z28.s

    // Accumulate Column 3: C_col3 += A * B[3]
    ld1rw z28.s, p0/z, [x1, #12]    // Load B[3]
    fmla z12.s, p0/m, z24.s, z28.s
    fmla z13.s, p0/m, z25.s, z28.s
    fmla z14.s, p0/m, z26.s, z28.s
    fmla z15.s, p1/m, z27.s, z28.s

    // Accumulate Column 4: C_col4 += A * B[4]
    ld1rw z28.s, p0/z, [x1, #16]    // Load B[4]
    fmla z16.s, p0/m, z24.s, z28.s
    fmla z17.s, p0/m, z25.s, z28.s
    fmla z18.s, p0/m, z26.s, z28.s
    fmla z19.s, p1/m, z27.s, z28.s

    // Accumulate Column 5: C_col5 += A * B[5]
    ld1rw z28.s, p0/z, [x1, #20]    // Load B[5]
    fmla z20.s, p0/m, z24.s, z28.s
    fmla z21.s, p0/m, z25.s, z28.s
    fmla z22.s, p0/m, z26.s, z28.s
    fmla z23.s, p1/m, z27.s, z28.s

    // Advance the input and output pointers
    add x0, x0, #124
    add x1, x1, #24

    // Decrement the loop counter 'c'
    sub w3, w3, #1

    // Branch to .Lloop if w3 is not zero (Compare and Branch on Non-Zero)
    cbnz w3, .Lloop

.Lend:
    // Reset x5 to the start of C
    mov x5, x2

    st1w z0.s, p0, [x5, #0, mul vl]
    st1w z1.s, p0, [x5, #1, mul vl]
    st1w z2.s, p0, [x5, #2, mul vl]
    st1w z3.s, p1, [x5, #3, mul vl]
    add x5, x5, #124

    st1w z4.s, p0, [x5, #0, mul vl]
    st1w z5.s, p0, [x5, #1, mul vl]
    st1w z6.s, p0, [x5, #2, mul vl]
    st1w z7.s, p1, [x5, #3, mul vl]
    add x5, x5, #124

    st1w z8.s, p0, [x5, #0, mul vl]
    st1w z9.s, p0, [x5, #1, mul vl]
    st1w z10.s, p0, [x5, #2, mul vl]
    st1w z11.s, p1, [x5, #3, mul vl]
    add x5, x5, #124

    st1w z12.s, p0, [x5, #0, mul vl]
    st1w z13.s, p0, [x5, #1, mul vl]
    st1w z14.s, p0, [x5, #2, mul vl]
    st1w z15.s, p1, [x5, #3, mul vl]
    add x5, x5, #124

    st1w z16.s, p0, [x5, #0, mul vl]
    st1w z17.s, p0, [x5, #1, mul vl]
    st1w z18.s, p0, [x5, #2, mul vl]
    st1w z19.s, p1, [x5, #3, mul vl]
    add x5, x5, #124

    st1w z20.s, p0, [x5, #0, mul vl]
    st1w z21.s, p0, [x5, #1, mul vl]
    st1w z22.s, p0, [x5, #2, mul vl]
    st1w z23.s, p1, [x5, #3, mul vl]

    // EPILOGUE: Restore callee-saved registers
    ldp d14, d15, [sp, #48]
    ldp d12, d13, [sp, #32]
    ldp d10, d11, [sp, #16]
    ldp d8, d9, [sp], #64

    ret
