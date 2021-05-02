// CI: Test if zero register works properly

__attribute__ ((naked)) void test_main() {
    __asm__ volatile (
        "test_r0:                 \n"
        "li $s0, 0x1f802082       \n" // Pointer to exit register in $s0
        "lui $s1, 0               \n" // 0 in $s1
        "lui $zero, 10            \n" // Try storing a non-zero value into $zero
        "bne $zero, $s1, .failure \n" // Check if $zero == $s1
        "sb $s1, 0($s0)           \n" // If passed, write 0 to exit register
        ".failure:                \n" 
        "li $s1, 1                \n" // If it didn't passed, write 1 to exit register
        "sb $s1, 0($s0)           \n"
        ".lock:                   \n"
        "j .lock"
    );
}

int main() {
    test_main();
}