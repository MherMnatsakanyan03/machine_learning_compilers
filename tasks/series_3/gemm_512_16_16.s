.text
    .global _gemm_512_16_16

_gemm_512_16_16:
    // Task: implement C = C + A * B gemm for k = 512; m = 16; n = 16
    // Arguments: x0 = a, x1 = b, x2 = c, w3 = k (512)

    cmp w3, #0
    ble .Lend

    // Save frame pointer & link register
    stp x29, x30, [sp, #-16]!
    mov x29, sp

    smstart
    ptrue p0.s

    // load C
    mov x5, x2
    mov w12, #0
.Lload_c:
    // Load 16 floats into column 'w12' of za0
    ld1w {za0v.s[w12, 0]}, p0/z, [x5]
    // Advance 16 floats (64 bytes) to the next column
    add x5, x5, #64
    // advance to the "next" column in za
    add w12, w12, #1
    cmp w12, #16
    b.lt .Lload_c

    // accumulate A * B
    // Unroll the loop by 4 (512 / 4 = 128 iterations)
    // (we divide the 512 by 4 by left-shifting 2 bits)
    lsr w3, w3, #2          
.Lmain_loop:
    // Load 4 columns of A
    ld1w z0.s, p0/z, [x0, #0, mul vl]
    ld1w z1.s, p0/z, [x0, #1, mul vl]
    ld1w z2.s, p0/z, [x0, #2, mul vl]
    ld1w z3.s, p0/z, [x0, #3, mul vl]
    add x0, x0, #256

    // Load 4 rows of B
    ld1w z4.s, p0/z, [x1, #0, mul vl]
    ld1w z5.s, p0/z, [x1, #1, mul vl]
    ld1w z6.s, p0/z, [x1, #2, mul vl]
    ld1w z7.s, p0/z, [x1, #3, mul vl]
    add x1, x1, #256

    // multiply col of A and row of B, and add 16x16 grid to za0
    fmopa za0.s, p0/m, p0/m, z0.s, z4.s
    fmopa za0.s, p0/m, p0/m, z1.s, z5.s
    fmopa za0.s, p0/m, p0/m, z2.s, z6.s
    fmopa za0.s, p0/m, p0/m, z3.s, z7.s

    sub w3, w3, #1
    cbnz w3, .Lmain_loop

    // store C
    mov x5, x2
    mov w12, #0

.Lstore_c:
    st1w {za0v.s[w12, 0]}, p0, [x5]
    add x5, x5, #64
    add w12, w12, #1
    cmp w12, #16
    b.lt .Lstore_c

    smstop

    ldp x29, x30, [sp], #16

.Lend:
    ret
