# OSH 实验 1-3: 利用汇编实现 LED 闪烁

**Keyu Tao, PB17111630**

---

## 使 LED 灯闪烁的 ARM 汇编程序

目前我们知道，这个程序是能把灯点亮的：

```assembly
.section .init
.global _start
_start:

ldr r0, =0x3F200000  @ GPIO Controller addr

mov r1, #1
lsl r1, #27
str r1, [r0, #8]  @ Set LED to Output mode

mov r1, #1
lsl r1, #29
str r1, [r0, #28]  @ Open LED
```

而我们要求的程序需要把 LED 闪烁起来，间隔 2 秒。

那么：

1. 设置 GPIO 控制器地址和设置 LED 为输出模式的代码需要保留下来。
2. 需要依靠计时器控制器实现「忙等」指定的时间。

### 问题 1: 如何关闭 LED 灯

从直觉来看，把 11～13 行的 `r1` 换成 0 就能把灯关掉，但实际上是不行的。在要求中也提到了：

> GPIO 40-48 这 8 个字节偏移中每个 bit 分别对应一个端口，将某一 bit 设置为 1，则会将对应的端口关闭。

而 BCM2837 的手册也提到了：

| Address | Field Name | Description | Size | Read/Write |
| ------------ | ------ | ----------------------- | ---- | ---- |
| 0x 7E20 001C | GPSET0 | GPIO Pin Output Set 0   | 32   | W    |
| 0x 7E20 0020 | GPSET1 | GPIO Pin Output Set 1   | 32   | W    |
| 0x 7E20 0024 | -      | Reserved                | -    | -    |
| 0x 7E20 0028 | GPCLR0 | GPIO Pin Output Clear 0 | 32   | W    |
| 0x 7E20 002C | GPCLR1 | GPIO Pin Output Clear 1 | 32   | W    |

*（注：手册里提到的基址似乎是错的）*

所以为了关闭 LED，需要把 13 行的 `#28` 改成 `#40`。

### 问题 2: 关于打开与关闭 LED 的间隔

如果在打开 LED 后马上关闭 LED 会产生什么？这样的效果不是 LED 灯闪烁，而是 LED 不亮，因为其中需要一定的时间间隔。

---

所以现在，我们可以写出这个程序的大致思路：

```assembly
@ 设置 LED 输出模式

loop:
@ 等 1 秒
@ 灯亮
@ 等 1 秒
@ 灯灭
b loop
```

灯亮和灯灭的代码上面已经完成了。下面要做的是等待 1 秒的代码。已知用下面这段代码，可以读取计时器低 4 字节：

```assembly
ldr r2, =0x3F003000
ldr r3, [r2, #4]   @ 读取计时器的低 4 字节
```

已知这个计时器是 1MHz 的，过 1s 时增加值为 $10^6$。那么计时的大致思路是：

```assembly
@ 读取计时器
@ x = 读取的值 + 1000000
loop:
@ 读取计时器
@ 读取值与 x 比较
ble loop  @ 小于等于 x 就继续
```

这里会遇到最后一个坑点：ARM 中的立即数。

### 问题 3: ARM 的立即数

ARM 对立即数值要求是有限制的。在 ARM 中，留给立即数的只有 12 位，看似只能表示 0 - 4095 的值。但在 ARM 中，12 位被拆成了两部分：8 位以加载 0 - 255 的值，4 位乘以 2 后表示循环右移的步数（0 到 30）。这样可以表示大范围的数，但在这个范围中的数不是所有的都能表示。

所以当写出类似于 `mov r0, #xxx` 这样的代码时，不一定能汇编成功。一种方法是 `ldr r0, =xxx` 以在 `r0` 中加载一个数。这种方式会从常量池（Literal pool）中加载这个数给 `r0`。

---

最后，我们希望把「等 1 秒」做成一个函数/子过程，减少代码重复，但是不想调用的时候对栈做多余的操作。此时先可以使用 `Branch Link` 指令（`BL`），这会将返回地址放入 `link register`（`lr`），然后跳转。子过程执行结束时 `mov pc, lr` 即可。

---

由是可得最终的代码：

```assembly
.section .init
.global _start
_start:

ldr r0, =0x3F200000

mov r1, #0x8000000
str r1, [r0, #8]  @ set output mode

ldr r2, =0x3F003000
ldr r3, =1000000  @ 1 second
mov r4, #0x20000000

on:
bl wait_one_sec

str r4, [r0, #28]

bl wait_one_sec

off:
str r4, [r0, #40]

b on

wait_one_sec:
ldr r1, [r2, #4]
add r1, r1, r3
loop:
ldr r5, [r2, #4]
cmp r5, r1
ble loop
mov pc, lr
```

## `/dev/sdc1` 中重要的文件

对于实验 1-3，我们最小只需要三个文件： `kernel7.img` 、`bootcode.bin` 和 `start.elf` 。`bootcode.bin` 和 `start.elf` 的作用见实验 1-1 报告。

## `/dev/sdc2` 是否被用到

很明显没有，毕竟我们的汇编也没写读 `ext4` 文件系统的功能呀。

## `as`, `ld`, `objcopy` 的作用

`as` 是 assembler，负责将汇编转换为目标文件，以供下一步链接使用。

`ld` 是 linker，负责将目标文件与库链接，生成可执行文件。

`objcopy` 负责对目标文件进行处理，复制目标文件的一部分内容到另一个文件。

在此实验中，`as` 生成的 `led.o` 如下：

```
[tao@tao-linux-vmware lab1]$ file led.o
led.o: ELF 32-bit LSB relocatable, ARM, EABI5 version 1 (SYSV), not stripped
[tao@tao-linux-vmware lab1]$ arm-linux-gnueabihf-objdump -d led.o

led.o：     文件格式 elf32-littlearm


Disassembly of section .init:

00000000 <_start>:
   0:	e59f003c 	ldr	r0, [pc, #60]	; 44 <loop+0x10>
   4:	e3a01302 	mov	r1, #134217728	; 0x8000000
   8:	e5801008 	str	r1, [r0, #8]
   c:	e59f2034 	ldr	r2, [pc, #52]	; 48 <loop+0x14>
  10:	e59f3034 	ldr	r3, [pc, #52]	; 4c <loop+0x18>
  14:	e3a04202 	mov	r4, #536870912	; 0x20000000

00000018 <on>:
  18:	eb000003 	bl	2c <wait_one_sec>
  1c:	e580401c 	str	r4, [r0, #28]
  20:	eb000001 	bl	2c <wait_one_sec>

00000024 <off>:
  24:	e5804028 	str	r4, [r0, #40]	; 0x28
  28:	eafffffa 	b	18 <on>

0000002c <wait_one_sec>:
  2c:	e5921004 	ldr	r1, [r2, #4]
  30:	e0811003 	add	r1, r1, r3

00000034 <loop>:
  34:	e5925004 	ldr	r5, [r2, #4]
  38:	e1550001 	cmp	r5, r1
  3c:	dafffffc 	ble	34 <loop>
  40:	e1a0f00e 	mov	pc, lr
  44:	3f200000 	.word	0x3f200000
  48:	3f003000 	.word	0x3f003000
  4c:	000f4240 	.word	0x000f4240
```

基本是我们汇编的内容。而在链接之后：

```
[tao@tao-linux-vmware lab1]$ arm-linux-gnueabihf-objdump -d led.elf

led.elf：     文件格式 elf32-littlearm


Disassembly of section .init:

00008054 <_start>:
    8054:	e59f003c 	ldr	r0, [pc, #60]	; 8098 <loop+0x10>
    8058:	e3a01302 	mov	r1, #134217728	; 0x8000000
    805c:	e5801008 	str	r1, [r0, #8]
    8060:	e59f2034 	ldr	r2, [pc, #52]	; 809c <loop+0x14>
    8064:	e59f3034 	ldr	r3, [pc, #52]	; 80a0 <loop+0x18>
    8068:	e3a04202 	mov	r4, #536870912	; 0x20000000

0000806c <on>:
    806c:	eb000003 	bl	8080 <wait_one_sec>
    8070:	e580401c 	str	r4, [r0, #28]
    8074:	eb000001 	bl	8080 <wait_one_sec>

00008078 <off>:
    8078:	e5804028 	str	r4, [r0, #40]	; 0x28
    807c:	eafffffa 	b	806c <on>

00008080 <wait_one_sec>:
    8080:	e5921004 	ldr	r1, [r2, #4]
    8084:	e0811003 	add	r1, r1, r3

00008088 <loop>:
    8088:	e5925004 	ldr	r5, [r2, #4]
    808c:	e1550001 	cmp	r5, r1
    8090:	dafffffc 	ble	8088 <loop>
    8094:	e1a0f00e 	mov	pc, lr
    8098:	3f200000 	.word	0x3f200000
    809c:	3f003000 	.word	0x3f003000
    80a0:	000f4240 	.word	0x000f4240
```

可以注意到一些微小的变化，基址从 0 变成了 0x8000。而 `objcopy` 就将生成的 ELF 中的指令复制了出来，以让 `start.elf` 加载。

```
[tao@tao-linux-vmware lab1]$ file led.img
led.img: data
[tao@tao-linux-vmware lab1]$ hexdump led.img
0000000 003c e59f 1302 e3a0 1008 e580 2034 e59f
0000010 3034 e59f 4202 e3a0 0003 eb00 401c e580
0000020 0001 eb00 4028 e580 fffa eaff 1004 e592
0000030 1003 e081 5004 e592 0001 e155 fffc daff
0000040 f00e e1a0 0000 3f20 3000 3f00 4240 000f
0000050
```

可以看到，`led.img` 中就是纯粹的指令，没有 ELF 文件结构。

日常编译程序需要 `as` 和 `ld`，以生成可以直接执行的文件。`objcopy` 不需要，因为我们写的程序为了让操作系统运行，要按照一定的文件格式（对于 Linux 是 ELF），不需要使用 `objcopy` 再次处理。

## References

*（除材料中已经给出的外）*

1. [5.3.3. Calling Subroutines](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0040d/Cihcfigg.html)