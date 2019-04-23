CFLAGS:=-g -Wall -Wextra $(shell pkg-config --cflags ic_barcode)

.PHONY: all clean

all: ic-barcode-cli

ic-barcode-cli: main.cpp
	g++ -std=gnu++17 $(CFLAGS) main.cpp -o ic-barcode-cli -lic_barcode

clean:
	rm ic-barcode-cli

