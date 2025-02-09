/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2003 Eric Biederman
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

#include <console/console.h>
#include <console/cbmem_console.h>
#include <console/uart.h>
#include <console/usb.h>
#include <console/ne2k.h>
#include <console/spkmodem.h>

#ifndef __PRE_RAM__

/* initialize the console */
void console_hw_init(void)
{
	struct console_driver *driver;

	for(driver = console_drivers; driver < econsole_drivers; driver++) {
		if (!driver->init)
			continue;
		driver->init();
	}
}

void console_tx_flush(void)
{
	struct console_driver *driver;
	for(driver = console_drivers; driver < econsole_drivers; driver++) {
		if (!driver->tx_flush)
			continue;
		driver->tx_flush();
	}
}

void console_tx_byte(unsigned char byte)
{
	struct console_driver *driver;
	for(driver = console_drivers; driver < econsole_drivers; driver++) {
		driver->tx_byte(byte);
	}
}

unsigned char console_rx_byte(void)
{
	struct console_driver *driver;
	for(driver = console_drivers; driver < econsole_drivers; driver++) {
		if (driver->tst_byte)
			break;
	}
	if (driver == econsole_drivers)
		return 0;
	while (!driver->tst_byte());
	return driver->rx_byte();
}

int console_tst_byte(void)
{
	struct console_driver *driver;
	for(driver = console_drivers; driver < econsole_drivers; driver++)
		if (driver->tst_byte)
			return driver->tst_byte();
	return 0;
}

#else // __PRE_RAM__   ^^^ NOT defined   vvv defined

void console_hw_init(void)
{
#if CONFIG_CONSOLE_SERIAL
	uart_init();
#endif
#if CONFIG_CONSOLE_NE2K
	ne2k_init(CONFIG_CONSOLE_NE2K_IO_PORT);
#endif
#if CONFIG_CONSOLE_CBMEM && CONFIG_EARLY_CBMEM_INIT && !defined(__BOOT_BLOCK__)
	cbmemc_init();
#endif
#if CONFIG_SPKMODEM
	spkmodem_init();
#endif
#if CONFIG_CONSOLE_USB && CONFIG_USBDEBUG_IN_ROMSTAGE && !defined(__BOOT_BLOCK__)
	usbdebug_init();
#endif
}
#endif
