// CI: Test if sign extension in CPU instructions works properly
// TODO: Test remaining sign extending ops

__attribute__ ((naked)) void test_main() {
    __asm__ volatile (R"(
        test_start:
        li $s0, 0x1f802082  # Pointer to exit register in $s0
        li $t0, 0xFFFFFFFF  # Set $t1 to 0xFFFFFFFF
        move $t1, $zero     # Set $t0 to 0
        addiu $t1, 0xFFFF   # Add 0xFFFF (should set $t1 to 0xFFFF'FFFF due to extension)
        bne $t0, $t1, .failure  # Fail if $t1 is not 0xFFFF'FFFF
        nop

        sw $t0, 0($sp)     # Write 0xFFFF'FFFF to stack
        addiu $sp, 4       # Add 4 to stack pointer
        lw $t1, -4($sp)    # Test a backwards 
        nop
        bne $t0, $t1, .failure # Fail if it didn't work
        nop

        sw $s0, -4($sp)    # Test a backwards sw
        lw $t1, -4($sp)
        nop
        bne $s0, $t1, .failure
        nop

        .success:          # Ran if all tests passed
            li $s1, 0     
            sb $s1, 0($s0)
        .failure: 
            li $s1, 1      # If it didn't passed, write 1 to exit register
            sb $s1, 0($s0)
        .lock:
            j .lock
            nop
    )");
}

int main() {
    test_main();
}