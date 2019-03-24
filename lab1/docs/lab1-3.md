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

## References

*（除材料中已经给出的外）*

1. [5.3.3. Calling Subroutines](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0040d/Cihcfigg.html)