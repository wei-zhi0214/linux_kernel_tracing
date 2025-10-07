# ==== 設定區 ====
TARGET      ?= fork
SRC         := fork.c
ROOTFS      ?=/home/william/linux-kernel/initramfs
# 預設用 gcc；如需交叉編譯可下：make CC=x86_64-linux-gnu-gcc
CC          ?= gcc
CFLAGS      ?= -O2 -Wall -Wextra -pipe
LDFLAGS     ?=
# 設 STATIC=1 會啟用靜態連結，方便用 qemu-user 跑而不依賴動態庫
ifeq ($(STATIC),1)
    CFLAGS  += -static
    LDFLAGS += -static
    OUT      = $(TARGET).static
else
    OUT      = $(TARGET)
endif

# ==== 規則 ====
.PHONY: all clean run-user

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).static

# 使用 qemu-user（qemu-x86_64）直接在主機上跑靜態檔
# 例：make STATIC=1 && make run-user
run-user: $(TARGET).static
	qemu-x86_64 ./$(TARGET).static
install: $(OUT)
	cp $(OUT) $(ROOTFS)/
	@echo "Installed $(OUT) to $(ROOTFS)/"
