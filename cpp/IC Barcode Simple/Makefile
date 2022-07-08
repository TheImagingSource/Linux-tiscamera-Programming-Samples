CC = gcc
GCCVERSIONGTEQ4 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \<= 5)

CFLAGS:=-g -Wall -Wextra $(shell pkg-config --cflags ic_barcode)

.PHONY: all clean

all: ic-barcode-cli

ic-barcode-cli: main.cpp
ifeq "$(GCCVERSIONGTEQ4)" "1"
	@echo "Your gcc version is too old. gcc 6 or higher is required."
	@echo "Please refer to https://askubuntu.com/questions/781972/how-can-i-update-gcc-5-3-to-6-1"
	@echo "Stopping"
else
	g++ -std=gnu++17 $(CFLAGS) main.cpp -o ic-barcode-cli -lic_barcode
endif
clean:
	rm ic-barcode-cli

