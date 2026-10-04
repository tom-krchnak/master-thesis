CMAKE ?= cmake
PRESET ?= release

IMPL_BUILD_DIR := impl/build/$(PRESET)
TOOLS_BUILD_DIR := tools/build/$(PRESET)
SALAC ?= $(or $(wildcard $(TOOLS_BUILD_DIR)/install/bin/salac.py),salac.py)
SALASYM := $(IMPL_BUILD_DIR)/salasym/salasym

TYPST ?= typst
THESIS_ROOT := text
THESIS_DIR := $(THESIS_ROOT)/salasym
THESIS_FONT_PATH := $(THESIS_ROOT)/template/assets/fonts
THESIS_OUT_DIR := $(THESIS_DIR)/build
THESIS_PDF := $(THESIS_OUT_DIR)/salasym.pdf

FILE ?=
FILE_STEM := $(basename $(notdir $(FILE)))
OUTPUT_DIR ?= impl/out/$(FILE_STEM)

.DEFAULT_GOAL := thesis
.PHONY: thesis watch init salasym salac verify patch-submodules unpatch-submodules clean

init:
	$(CMAKE) --preset $(PRESET) -S impl

salasym: init
	$(CMAKE) --build "$(IMPL_BUILD_DIR)" --target salasym

salac:
	$(CMAKE) --preset $(PRESET) -S tools
	$(CMAKE) --build "$(TOOLS_BUILD_DIR)" --target salac
	$(CMAKE) --install "$(TOOLS_BUILD_DIR)/binsalac"

thesis:
	@mkdir -p "$(THESIS_OUT_DIR)"
	$(TYPST) compile --root "$(THESIS_ROOT)" --font-path "$(THESIS_FONT_PATH)" "$(THESIS_DIR)/main.typ" "$(THESIS_PDF)"

watch:
	@mkdir -p "$(THESIS_OUT_DIR)"
	$(TYPST) watch --root "$(THESIS_ROOT)" --font-path "$(THESIS_FONT_PATH)" "$(THESIS_DIR)/main.typ" "$(THESIS_PDF)"

verify: salasym
	@test -n "$(FILE)" || { echo 'usage: `make verify FILE=<file>`'; exit 2; }
	@command -v "$(SALAC)" >/dev/null 2>&1 || { echo 'salac not found; run `make salac`, add salac.py to PATH, or set SALAC=/path/to/salac.py'; exit 2; }
	@$(RM) -r "$(OUTPUT_DIR)"
	@echo input: "$(FILE)"
	@echo ---
	@"$(SALAC)" --input "$(FILE)" --output "$(OUTPUT_DIR)" --opt 2
	@"$(SALASYM)" "$(OUTPUT_DIR)/$(FILE_STEM).json"

define apply_patch
	@if git -C "$(1)" apply --check "$(abspath $(2))" 2>/dev/null; then \
		git -C "$(1)" apply "$(abspath $(2))" && \
		printf 'Applied %s\n' "$(2)"; \
	elif git -C "$(1)" apply --reverse --check "$(abspath $(2))" 2>/dev/null; then \
		printf 'Already applied %s\n' "$(2)"; \
	else \
		printf 'Cannot apply %s to %s\n' "$(2)" "$(1)" >&2; \
		exit 1; \
	fi
endef

patch-submodules:
	$(call apply_patch,impl/libsala,patches/libsala-boost-json.patch)
	$(call apply_patch,tools/libllvmutl,patches/libllvmutl-llvm22.patch)
	$(call apply_patch,tools/binsalac,patches/binsalac-llvm22.patch)

unpatch-submodules:
	git submodule foreach --recursive 'git restore --source=HEAD --staged --worktree -- .'

clean:
	$(RM) -r impl/build tools/build
