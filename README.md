Jaster Kernel
=======

Это первое функциональное ядро Jaster Kernel с функцией калькулятора, в будущем функции будут дополняться



#### Команды сборки ####
```
nasm -f elf32 kernel.asm -o kasm.o
```
```
gcc -m32 -ffreestanding -nostdlib -fno-stack-protector -c kernel.c -o kernel.o
```
```
ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o
```

#### Test on emulator ####
```
qemu-system-i386 -kernel kernel
```

Воуля! у вас все работает!

![kernel screenshot](http://31.media.tumblr.com/1afd75b433b13df613fa0c2301977893/tumblr_inline_ncy1p0kSGj1rivrqc.png "Screenshot")
