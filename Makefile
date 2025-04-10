CC = gcc
CFLAGS = -Wall
TARGET = nemo
PREFIX = /usr/local/bin

all: install

$(TARGET): nemo.c
	$(CC) $(CFLAGS) nemo.c -o $(TARGET)

install: $(TARGET)
	sudo install -Dm755 $(TARGET) $(PREFIX)/$(TARGET)
	@echo "✅ Installed $(TARGET) to $(PREFIX)"

uninstall:
	sudo rm -f $(PREFIX)/$(TARGET)
	@echo "❌ Uninstalled $(TARGET) from $(PREFIX)"

clean:
	rm -f $(TARGET)
