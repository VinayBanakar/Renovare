# https://cseweb.ucsd.edu/classes/sp10/cse141/pdf/02/S01_x86_64.key.pdf

.data
    s:
        .ascii "hello world\n"
        len = . - s
.text
    .global _start
    _start:

        movl $4, %eax   /* write system call number */
        movl $1, %ebx   /* stdout */
        movl $s, %ecx   /* the data to print */
        movl $len, %edx /* length of the buffer */
        int $0x80

        movl $1, %eax   /* exit system call number */
        movl $0, %ebx   /* exit status */
        int $0x80

# as -o asm_syscall_2.o asm_syscall_2.s
# ld -o asm_syscall_2.out asm_syscall_2.o
# ./asm_syscall_2.out
