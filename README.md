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

![kernel screenshot](https://64.media.tumblr.com/5289a55bd22e8af99b30ad0a38bb50eb/cbd29f185ec1dfec-84/s540x810/ce88134fc83b9253cad561ddd73862626bd6f7de.png) "Screenshot")
