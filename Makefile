
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

COMPILER=/usr/local/lib/fpc/3.3.1/ppcx64
EXECUTABLE=`pwd`/build/pps
DEST=/usr/local/bin/pps
BUILD_DIR=`pwd`/build

all:
	mkdir -p $(BUILD_DIR)
	rm -f $(BUILD_DIR)/pps.o
	rm -f $(BUILD_DIR)/pps.ppu
	rm -f $(BUILD_DIR)/PasScript.o
	rm -f $(BUILD_DIR)/PasScript.ppu
	$(COMPILER) -FU"$(BUILD_DIR)" -o"$(EXECUTABLE)" pps.pas

install:
	rm -f $(DEST)
	ln -s "$(EXECUTABLE)" "$(DEST)"
