/*
 * Simple I2C example
 *
 * Copyright 2017 Joel Stanley <joel@jms.id.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}


static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     I2C_SMBUS_BYTE_DATA,&data))
		return -1;
	else
		return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value)
{
	union i2c_smbus_data data;
	data.byte = value;
	if (i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
	                     I2C_SMBUS_BYTE_DATA, &data))
		return -1;
	else
		return 0;
}

int main(int argc, char **argv)
{
	uint8_t read_data, write_data, addr = 0x76, reg = 0x0d;
	const char *path = argv[1];
	int file, rc;

	if (argc == 1)
		errx(-1, "path [i2c address] [register]");

	if (argc > 2)
		addr = strtoul(argv[2], NULL, 0);
	if (argc > 3)
		reg = strtoul(argv[3], NULL, 0);

	file = open(path, O_RDWR);
	if (file < 0)
		err(errno, "Tried to open '%s'", path);

	rc = ioctl(file, I2C_SLAVE, addr);
	if (rc < 0)
		err(errno, "Tried to set device address '0x%02x'", addr);

	// Reading from the specified register
	read_data = i2c_smbus_read_byte_data(file, reg);
	printf("%s: device 0x%02x at register 0x%02x: 0x%02x\n",
			path, addr, reg, read_data);

	// Writing a value to the specified register
	write_data = 0x68; // Example value to write
	rc = i2c_smbus_write_byte_data(file, reg, write_data);
	if (rc < 0)
		err(errno, "Failed to write to register 0x%02x", reg);
	else
		printf("Successfully wrote 0x%02x to register 0x%02x\n", write_data, reg);

	close(file);
}
