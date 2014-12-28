CFLAGS := -O0 -g \
	-D_GNU_SOURCE \
	-DTERMINAL_COLORS=\"terminal-colors.d\"

all: terminal-colors
terminal-colors: terminal-colors.o
