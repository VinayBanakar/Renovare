// if you want analyze cycles asm takes in the symbol you are interested in.
perf annotate -Mintel --stdio --symbol=__memmove_avx512_no_vzeroupper
