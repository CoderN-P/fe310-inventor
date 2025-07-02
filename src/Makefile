# Copyright 2019 SiFive, Inc #
# SPDX-License-Identifier: Apache-2.0 #

PROGRAM ?= example-i2c

SRCS := $(shell find . -name '*.c' -o -name '*.S')

$(PROGRAM): $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(PROGRAM) $(PROGRAM).hex
