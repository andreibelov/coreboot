# Run an intermediate step when producing coreboot.rom
# that adds additional components to the final firmware
# image outside of CBFS
INTERMEDIATE += exynos5420_add_bl1

bootblock-y += spi.c
bootblock-y += pinmux.c mct.c power.c
# Clock is required for UART
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += clock_init.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += clock.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += monotonic_timer.c
ifeq ($(CONFIG_DRIVERS_UART),y)
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += uart.c
endif
bootblock-y += wakeup.c
bootblock-y += gpio.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += timer.c

romstage-y += spi.c
romstage-y += clock.c
romstage-y += clock_init.c
romstage-y += pinmux.c  # required by s3c24x0_i2c and uart.
romstage-y += dmc_common.c
romstage-y += dmc_init_ddr3.c
romstage-y += power.c
romstage-y += mct.c
romstage-y += monotonic_timer.c
ifeq ($(CONFIG_DRIVERS_UART),y)
romstage-$(CONFIG_EARLY_CONSOLE) += uart.c
endif
romstage-y += wakeup.c
romstage-y += gpio.c
romstage-y += timer.c
romstage-y += i2c.c
#romstage-y += wdt.c
romstage-y += cbmem.c

ramstage-y += spi.c
ramstage-y += clock.c
ramstage-y += clock_init.c
ramstage-y += pinmux.c
ramstage-y += power.c
ramstage-$(CONFIG_DRIVERS_UART) += uart.c
ramstage-y += cpu.c
ramstage-y += tmu.c
ramstage-y += mct.c
ramstage-y += monotonic_timer.c
ramstage-y += timer.c
ramstage-y += gpio.c
ramstage-y += i2c.c
ramstage-y += dp.c dp_lowlevel.c fimd.c
ramstage-y += usb.c
ramstage-y += cbmem.c

exynos5420_add_bl1: $(obj)/coreboot.pre
	printf "    DD         Adding Samsung Exynos5420 BL1\n"
	dd if=3rdparty/cpu/samsung/exynos5420/bl1.bin \
		of=$(obj)/coreboot.pre conv=notrunc >/dev/null 2>&1
