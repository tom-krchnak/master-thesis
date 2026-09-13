CMAKE ?= cmake
BUILD_DIR ?= build
BUILD_TYPE ?= Release

INSTALL_DIR ?= $(abspath $(BUILD_DIR))/install
INSTALL_STAMP := $(INSTALL_DIR)/.salac-installed
SALASYM := $(BUILD_DIR)/salasym/salasym
SALASYM_STAMP := $(BUILD_DIR)/.salasym-built


NAME ?=
OUTPUT_DIR ?= out
PATCH_DIR := $(abspath patches)


.PHONY: patch salac compile verify clean

patch:
	@apply_patch() { \
		if git -C "$$1" apply --check "$$2" 2>/dev/null; then \
			git -C "$$1" apply "$$2"; \
			printf 'Applied %s\n' "$$2"; \
		elif git -C "$$1" apply --reverse --check "$$2" 2>/dev/null; then \
			printf 'Already applied %s\n' "$$2"; \
		else \
			printf 'Cannot apply %s to %s\n' "$$2" "$$1" >&2; \
			return 1; \
		fi; \
	}; \
	apply_patch libllvmutl "$(PATCH_DIR)/libllvmutl-llvm22.patch"; \
	apply_patch libsala "$(PATCH_DIR)/libsala-boost-json.patch"; \
	apply_patch binsalac "$(PATCH_DIR)/binsalac-llvm22.patch"
	$(RM) "$(INSTALL_STAMP)"

salac: $(INSTALL_STAMP)

$(INSTALL_STAMP): CMakeLists.txt Makefile
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DBINSALAC_INSTALL=ON
	$(CMAKE) --build $(BUILD_DIR) --parallel
	$(CMAKE) --install $(BUILD_DIR)
	touch $@

$(SALASYM_STAMP): salac
	$(CMAKE) --build $(BUILD_DIR) --target salasym-exec --parallel
	test -x "$(SALASYM)"
	touch $@

verify: compile $(SALASYM_STAMP)
	./$(SALASYM) "$(OUTPUT_DIR)/$(NAME)/$(NAME).json"

compile: salac
	@test -n "$(NAME)" || { printf '%s\n' 'Usage: make compile NAME=<source-name>'; exit 2; }
	$(RM) -r "$(OUTPUT_DIR)/$(NAME)"
	./$(BUILD_DIR)/install/bin/salac.py \
		--input ./data/$(NAME).c \
		--output $(OUTPUT_DIR)/$(NAME) \
		--opt 2

clean:
	$(RM) -r build
