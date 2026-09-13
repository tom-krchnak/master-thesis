CMAKE ?= cmake
PRESET ?= release

BUILD_DIR ?= ./build
SALAC   := $(BUILD_DIR)/$(PRESET)/install/bin/salac.py
SALASYM := $(BUILD_DIR)/$(PRESET)/salasym/salasym

FILE ?=
FILE_STEM := $(basename $(notdir $(FILE)))
OUTPUT_DIR ?= out/$(FILE_STEM)

PATCH_DIR := $(abspath patches)


.PHONY: patch init salasym verify

init:
	$(CMAKE) --preset debug
	$(CMAKE) --preset release

	$(CMAKE) --build --preset release --target salac
	$(CMAKE) --install $(BUILD_DIR)/release

salasym:
# 	$(CMAKE) --preset $(PRESET)
	$(CMAKE) --build --preset $(PRESET) --target salasym

verify:
	@test -f "$(SALAC)" || { echo 'salac not installed, run `make init`'; exit 2; }
	@test -f "$(SALASYM)" || { echo 'build salasym with `make salasym`'; exit 2; }
	@test -n "$(FILE)" || { echo 'usage: `make verify FILE=<file>`'; exit 2; }
	
	@$(RM) -r "$(OUTPUT_DIR)"
	@echo input: $(FILE)
	@echo ---

	@$(SALAC) --input "$(FILE)" --output "$(OUTPUT_DIR)" --opt 2
	@$(SALASYM) "$(OUTPUT_DIR)/$(FILE_STEM).json"

clean:
	$(RM) -r build

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
