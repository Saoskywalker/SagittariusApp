;皆为unsigned char指针处理
;例: extern unsigned char _binary_input_txt_start[];
 ld -r -b binary -o res.o ff.gif
objdump -x res.o
@pause