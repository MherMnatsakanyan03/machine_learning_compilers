    .text
    .type transpose_cab, %function
    .global transpose_cab
transpose_cab:
    // Arguments:
    // x0 = input pointer
    // x1 = output pointer
    // w2 = rows (a = 4) [Unused in ASM since kernel is hardcoded]
    // w3 = cols (b = 8) [Unused in ASM since kernel is hardcoded]
    // w4 = c (number of matrices to transpose)
    // Check if c <= 0. If so, exit early to avoid an infinite loop.
    cmp w4, #0
    ble .Lend

.Lloop:
    ldr q0, [x0]
    ldr q1, [x0, #16]
    ldr q2, [x0, #32]
    ldr q3, [x0, #48]
    ldr q4, [x0, #64]
    ldr q5, [x0, #80]
    ldr q6, [x0, #96]
    ldr q7, [x0, #112]

    zip1 v16.4s, v0.4s, v1.4s
    zip1 v17.4s, v2.4s, v3.4s
    zip1 v18.4s, v4.4s, v5.4s
    zip1 v19.4s, v6.4s, v7.4s

    zip1 v20.2d, v16.2d, v17.2d
    zip1 v21.2d, v18.2d, v19.2d
    zip2 v22.2d, v16.2d, v17.2d
    zip2 v23.2d, v18.2d, v19.2d

    zip2 v16.4s, v0.4s, v1.4s
    zip2 v17.4s, v2.4s, v3.4s
    zip2 v18.4s, v4.4s, v5.4s
    zip2 v19.4s, v6.4s, v7.4s

    zip1 v24.2d, v16.2d, v17.2d
    zip1 v25.2d, v18.2d, v19.2d
    zip2 v26.2d, v16.2d, v17.2d
    zip2 v27.2d, v18.2d, v19.2d

    str q20, [x1]
    str q21, [x1, #16]
    str q22, [x1, #32]
    str q23, [x1, #48]
    str q24, [x1, #64]
    str q25, [x1, #80]
    str q26, [x1, #96]
    str q27, [x1, #112]

    // Advance the input and output pointers by 128 bytes (32 floats * 4 bytes)
    add x0, x0, #128
    add x1, x1, #128

    // Decrement the loop counter 'c'
    sub w4, w4, #1

    // Branch to .Lloop if w4 is not zero (Compare and Branch on Non-Zero)
    cbnz w4, .Lloop

.Lend:
    ret
