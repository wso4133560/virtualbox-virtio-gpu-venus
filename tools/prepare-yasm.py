"""Add the warning class already probed by VirtualBox's Config.kmk.

YASM 1.3 treats all segment-register tokens as ignored in long mode, including
legal MOV instructions used by the hypervisor. Keep the warning enabled by
default, but let VBox disable only this class with -Wno-segreg-in-64bit.
Instruction encoding and Windows unwind directives are unchanged.
"""
from pathlib import Path
import sys

root = Path(sys.argv[1])
edits = [
    ("libyasm/errwarn.h",
     "    YASM_WARN_IMPLICIT_SIZE_OVERRIDE /**< Implicit size override */",
     "    YASM_WARN_IMPLICIT_SIZE_OVERRIDE, /**< Implicit size override */\n"
     "    YASM_WARN_SEGREG_IN_64BIT /**< Segment register token in long mode */"),
    ("libyasm/errwarn.c",
     "        (1UL<<YASM_WARN_IMPLICIT_SIZE_OVERRIDE);",
     "        (1UL<<YASM_WARN_IMPLICIT_SIZE_OVERRIDE) | (1UL<<YASM_WARN_SEGREG_IN_64BIT);"),
    # Stock YASM prints this error banner even when there are no diagnostics.
    ("libyasm/errwarn.c",
     "    if (warning_as_error && warning_as_error != 2) {",
     "    if (warning_as_error && warning_as_error != 2 && yasm_errwarns_num_errors(errwarns, 1)) {"),
    ("frontends/yasm/yasm.c",
     '    else if (strcmp(cmd, "size-override") == 0)\n        action(YASM_WARN_SIZE_OVERRIDE);',
     '    else if (strcmp(cmd, "size-override") == 0)\n        action(YASM_WARN_SIZE_OVERRIDE);\n'
     '    else if (strcmp(cmd, "segreg-in-64bit") == 0)\n        action(YASM_WARN_SEGREG_IN_64BIT);'),
    ("modules/arch/x86/x86regtmod.gperf",
     '        yasm_warn_set(YASM_WARN_GENERAL,\n                      N_("`%s\' segment register ignored',
     '        yasm_warn_set(YASM_WARN_SEGREG_IN_64BIT,\n                      N_("`%s\' segment register ignored'),
]
# CMake 3/4 resolve generated tools at build time instead of reading LOCATION.
for target in ("genperf", "re2c", "genmacro"):
    variable = f"_tmp_{target.upper()}_EXE"
    edits.append(("cmake/modules/YasmMacros.cmake",
                  f"get_target_property({variable} {target} LOCATION)",
                  f"set({variable} $<TARGET_FILE:{target}>)"))
for name, before, after in edits:
    path = root / name
    content = path.read_text(encoding="utf-8")
    if after in content:
        continue
    if content.count(before) != 1:
        raise ValueError(f"Unexpected YASM source: {path}")
    path.write_text(content.replace(before, after), encoding="utf-8", newline="\n")
print("YASM warning class ready")
