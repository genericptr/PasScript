
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

COMPILER=/usr/local/lib/fpc/3.3.1/ppcx64
EXECUTABLE=`pwd`/build/pps
DEST=/usr/local/bin/pps
BUILD_DIR=`pwd`/build

all:
	mkdir -p $(BUILD_DIR)
	$(COMPILER) -FU"$(BUILD_DIR)" -o"$(EXECUTABLE)" pps.pas

install:
	rm -f $(DEST)
	ln -s "$(EXECUTABLE)" "$(DEST)"
