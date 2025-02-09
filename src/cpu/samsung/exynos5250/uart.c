/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <console/uart.h>
#include <arch/io.h>
#include <boot/coreboot_tables.h>
#include "uart.h"
#include "clk.h"
#include "cpu.h"
#include "periph.h"

#define RX_FIFO_COUNT_MASK	0xff
#define RX_FIFO_FULL_MASK	(1 << 8)
#define TX_FIFO_FULL_MASK	(1 << 24)


/*
 * The coefficient, used to calculate the baudrate on S5P UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recommend using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

static void serial_setbrg_dev(struct s5p_uart *uart)
{
	u32 uclk;
	u32 val;

	// All UARTs share the same clock.
	uclk = clock_get_periph_rate(PERIPH_ID_UART3);
	val = uclk / default_baudrate();

	writel(val / 16 - 1, &uart->ubrdiv);

	/*
	 * FIXME(dhendrix): the original uart.h had a "br_rest" value which
	 * does not seem relevant to the exynos5250... not entirely sure
	 * where/if we need to worry about it here
	 */
#if 0
	if (s5p_uart_divslot())
		writel(udivslot[val % 16], &uart->rest.slot);
	else
		writeb(val % 16, &uart->rest.value);
#endif
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static void exynos5_init_dev(struct s5p_uart *uart)
{
	// TODO initialize with correct peripheral id by base_port.
	exynos_pinmux_uart3();

	/* enable FIFOs */
	writel(0x1, &uart->ufcon);
	writel(0, &uart->umcon);
	/* 8N1 */
	writel(0x3, &uart->ulcon);
	/* No interrupts, no DMA, pure polling */
	writel(0x245, &uart->ucon);

	serial_setbrg_dev(uart);
}

static int exynos5_uart_err_check(struct s5p_uart *uart, int op)
{
	unsigned int mask;

	/*
	 * UERSTAT
	 * Break Detect	[3]
	 * Frame Err	[2] : receive operation
	 * Parity Err	[1] : receive operation
	 * Overrun Err	[0] : receive operation
	 */
	if (op)
		mask = 0x8;
	else
		mask = 0xf;

	return readl(&uart->uerstat) & mask;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is successful, the character read is
 * written into its argument c.
 */
static unsigned char exynos5_uart_rx_byte(struct s5p_uart *uart)
{
	/* wait for character to arrive */
	while (!(readl(&uart->ufstat) & (RX_FIFO_COUNT_MASK |
					 RX_FIFO_FULL_MASK))) {
		if (exynos5_uart_err_check(uart, 0))
			return 0;
	}

	return readb(&uart->urxh) & 0xff;
}

/*
 * Output a single byte to the serial port.
 */
static void exynos5_uart_tx_byte(struct s5p_uart *uart, unsigned char data)
{
	/* wait for room in the tx FIFO */
	while ((readl(&uart->ufstat) & TX_FIFO_FULL_MASK)) {
		if (exynos5_uart_err_check(uart, 1))
			return;
	}

	writeb(data, &uart->utxh);
}

static void exynos5_uart_tx_flush(struct s5p_uart *uart)
{
	while (readl(&uart->ufstat) & 0x1ff0000);
}

unsigned int uart_platform_base(int idx)
{
	return CONFIG_CONSOLE_SERIAL_UART_ADDRESS;
}

void uart_init(void)
{
	struct s5p_uart *uart = uart_platform_baseptr(0);
	exynos5_init_dev(uart);
}

unsigned char uart_rx_byte(void)
{
	struct s5p_uart *uart = uart_platform_baseptr(0);
	return exynos5_uart_rx_byte(uart);
}

void uart_tx_byte(unsigned char data)
{
	struct s5p_uart *uart = uart_platform_baseptr(0);
	exynos5_uart_tx_byte(uart, data);
}

void uart_tx_flush(void)
{
	struct s5p_uart *uart = uart_platform_baseptr(0);
	exynos5_uart_tx_flush(uart);
}

#ifndef __PRE_RAM__
void uart_fill_lb(void *data)
{
	struct lb_serial serial;
	serial.type = LB_SERIAL_TYPE_MEMORY_MAPPED;
	serial.baseaddr = uart_platform_base(0);
	serial.baud = default_baudrate();
	lb_add_serial(&serial, data);

	lb_add_console(LB_TAG_CONSOLE_SERIAL8250MEM, data);
}
#endif
