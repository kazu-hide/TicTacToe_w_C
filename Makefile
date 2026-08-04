CC := gcc

# ヘッダーファイルのディレクトリ
INCLUDE := include 

# コンパイルオプション
CFLAGS := -Wall -Wextra -Werror -I$(INCLUDE)

TARGET := Game
BUILD_DIR := build

COMMON_SRCS := src/board.c src/game.c src/ui.c src/queue.c src/cpu.c src/utils.c
SRCS := src/main.c $(COMMON_SRCS)
OBJS := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)


all: $(TARGET)

# 実行ファイルの生成
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -rf $(TEST_BUILD_DIR) $(TEST_TARGET)


TEST_TARGET := tests/build/test
TEST_BUILD_DIR := tests/build

TEST_SRCS := tests/test_main.c tests/test_runner.c tests/test_utils.c tests/test_board.c tests/test_ui.c tests/test_game.c tests/test_queue.c tests/test_cpu.c
TEST_OBJS := $(TEST_SRCS:tests/%.c=$(TEST_BUILD_DIR)/%.o) $(COMMON_SRCS:src/%.c=$(TEST_BUILD_DIR)/%.o)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $(TEST_TARGET)

$(TEST_BUILD_DIR)/%.o: tests/%.c | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BUILD_DIR)/%.o: src/%.c | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

test: clean $(TEST_TARGET)
	./$(TEST_TARGET)


# --- フォーマット (.clang-format を使用) ---
# 要 clang-format: brew install clang-format
FMT_SRCS := $(shell find src include tests chat -name '*.c' -o -name '*.h')

fmt:
	clang-format -i $(FMT_SRCS)

# 整形が必要なファイルがあれば非 0 で終わる (CI 用)
fmt-check:
	clang-format --dry-run --Werror $(FMT_SRCS)

.PHONY: all clean test fmt fmt-check
