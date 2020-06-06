	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 10, 15	sdk_version 10, 15
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$112, %rsp
	xorl	%eax, %eax
	movl	%eax, %ecx
	movq	___stack_chk_guard@GOTPCREL(%rip), %rdx
	movq	(%rdx), %rdx
	movq	%rdx, -8(%rbp)
	movl	$0, -52(%rbp)
	movl	%edi, -56(%rbp)
	movq	%rsi, -64(%rbp)
	leaq	-80(%rbp), %rdi
	movq	%rcx, %rsi
	callq	_gettimeofday
	leaq	-80(%rbp), %rdi
	leaq	-48(%rbp), %rcx
	movl	%eax, -84(%rbp)         ## 4-byte Spill
	movq	%rcx, -96(%rbp)         ## 8-byte Spill
	callq	_localtime
	movq	-96(%rbp), %rdi         ## 8-byte Reload
	movl	$40, %esi
	leaq	L_.str(%rip), %rdx
	movq	%rax, %rcx
	callq	_strftime
	leaq	-48(%rbp), %rsi
	leaq	L_.str.1(%rip), %rdi
	movq	%rax, -104(%rbp)        ## 8-byte Spill
	movb	$0, %al
	callq	_printf
	movq	___stack_chk_guard@GOTPCREL(%rip), %rcx
	movq	(%rcx), %rcx
	movq	-8(%rbp), %rdx
	cmpq	%rdx, %rcx
	movl	%eax, -108(%rbp)        ## 4-byte Spill
	jne	LBB0_2
## %bb.1:
	xorl	%eax, %eax
	addq	$112, %rsp
	popq	%rbp
	retq
LBB0_2:
	callq	___stack_chk_fail
	ud2
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Current date/time: %m-%d-%Y/%T"

L_.str.1:                               ## @.str.1
	.asciz	"%s\n"


.subsections_via_symbols
