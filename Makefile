CFLAGS := -DTERMINAL_COLORS=\"terminal-colors.d\"

all: terminal-colors
terminal-colors: terminal-colors.o
