// CI: Test if sign extension in CPU instructions works properly
// TODO: Test remaining sign extending ops

__attribute__ ((naked)) void test_main() {
    __asm__ volatile (
        "test_start:                 \n"
        "li $s0, 0x1f802082          \n" // Pointer to exit register in $s0
        "li $t0, 0xFFFFFFFF          \n" // Set $t1 to 0xFFFFFFFF
        "move $t1, $zero             \n" // Set $t0 to 0
        "addiu $t1, 0xFFFF           \n" // Add 0xFFFF (should set $t1 to 0xFFFF'FFFF due to extension)
        "bne $t0, $t1, .failure      \n" // Fail if $t1 is not 0xFFFF'FFFF

        "sw $t0, 0($sp)              \n" // Write 0xFFFF'FFFF to stack
        "addiu $sp, 4                \n" // Add 4 to stack pointer
        "lw $t1, -4($sp)             \n" // Test a backwards lw
        "bne $t0, $t1, .failure      \n" // Fail if it didn't work
        
        "sw $s0, -4($sp)             \n" // Test a backwards sw
        "lw $t1, -4($sp)             \n"
        "bne $s0, $s1, .failure      \n"

        ".success:                   \n" // Ran if all tests passed
        "    li $s1, 0               \n"
        "    sb $s1, 0($s0)          \n"
        ".failure:                   \n" 
        "    li $s1, 1               \n" // If it didn't passed, write 1 to exit register
        "    sb $s1, 0($s0)          \n"
        ".lock:                      \n"
        "    j .lock"
    );
}

int main() {
    test_main();
}