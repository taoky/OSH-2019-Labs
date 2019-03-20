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
