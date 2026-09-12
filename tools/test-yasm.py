"""Regression checks for the local YASM warning-class patch."""
from pathlib import Path
import subprocess
import sys

exe = Path(sys.argv[1]).resolve()
probe = exe.parent / "warning-probe.asm"
output = exe.parent / "warning-probe.bin"


def assemble(*flags):
    return subprocess.run([str(exe), "-f", "bin", *flags, "-o", str(output), str(probe)], capture_output=True)


probe.write_text("BITS 64\nmov eax, ds\nret\n")
assert assemble("-Werror").returncode != 0, "Default warning must remain enabled"
result = assemble("-Werror", "-Wno-segreg-in-64bit")
assert result.returncode == 0 and not result.stderr, result.stderr
assert output.read_bytes() == bytes.fromhex("8c d8 c3"), "Unexpected instruction encoding"
probe.write_text("BITS 64\nlabel_without_colon\nret\n")
assert assemble("-Werror", "-Worphan-labels", "-Wno-segreg-in-64bit").returncode != 0
probe.write_text("BITS 64\nthis_is_not_an_instruction eax\n")
assert assemble("-Wno-segreg-in-64bit").returncode != 0
print("YASM regression: PASS (machine code, default warning, unrelated warning, invalid instruction)")
