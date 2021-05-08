// CI: Test if zero register works properly

__attribute__ ((naked)) void test_main() {
    __asm__ volatile (R"(
        test_r0:
        li $s0, 0x1f802082   # Pointer to exit register in $s0
        lui $s1, 0           # 0 in $s1
        lui $zero, 10        # Try storing a non-zero value into $zero
        bne $zero, $s1, .failure # Check if $zero == $s1
        nop

        sb $s1, 0($s0)           # If passed, write 0 to exit register
        .failure:                # 
            li $s1, 1            # If it didn't passed, write 1 to exit register
            sb $s1, 0($s0)
        .lock:
            j .lock
            nop
    )");
}

int main() {
    test_main();
}