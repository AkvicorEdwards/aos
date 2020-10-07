#include "bootpack.h"
#include <stdio.h>

void init_pic(void) {
    io_out8(PIC0_IMR,  0xff  ); // 禁止所有中断
    io_out8(PIC1_IMR,  0xff  ); // 禁止所有中断

	io_out8(PIC0_ICW1, 0x11  ); // 边沿触发模式（edge trigger mode）
	io_out8(PIC0_ICW2, 0x20  ); // IRQ0-7 由 INT20-27接收
	io_out8(PIC0_ICW3, 1 << 2); // PIC1 由 IRQ2 连接
	io_out8(PIC0_ICW4, 0x01  ); // 无缓冲区模式

	io_out8(PIC1_ICW1, 0x11  ); // 边沿触发模式（edge trigger mode）
	io_out8(PIC1_ICW2, 0x28  ); // IRQ8-15 由 INT28-2f接收
	io_out8(PIC1_ICW3, 2     ); // PIC1 由 IRQ2 连接
	io_out8(PIC1_ICW4, 0x01  ); // 无缓冲区模式

	io_out8(PIC0_IMR,  0xfb  ); // 11111011 PIC1 以外全部禁止
	io_out8(PIC1_IMR,  0xff  ); // 11111111 禁止所有中断
}

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;

// 来自PS/2键盘的中断
void inthandler21(int *esp) {
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61); // 通知PIC"IRQ-01已经受理完毕"
    data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
}

struct FIFO8 mousefifo;

// 来自PS/2鼠标的中断
void inthandler2c(int *esp) {
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64); // 通知PIC IRQ-12 已经受理完毕
	io_out8(PIC0_OCW2, 0x62); // 通知PIC IRQ-02 已经受理完毕
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
}

// PIC0中断的不完整策略
// 这个中断在Athlon64X2上通过芯片组提供的便利，只需执行一次
// 这个中断只是接收，不执行任何操作
// 为什么不处理？
// 因为这个中断可能是电气噪声引发的、只是处理一些重要的情况。
void inthandler27(int *esp) {
	io_out8(PIC0_OCW2, 0x67); // 通知PIC的IRQ-07
	return;
}