"""Run statement examples and regression cases against a built executable."""
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
executable = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else root / "build/code").resolve()
statement = (root / "README.md").read_text()
cases = re.findall(r"\*\*Input:\*\*\s*```\n(.*?)\n```\s*\*\*Output:\*\*\s*```\n(.*?)\n```", statement, re.S)
cases += [
    ("9/-6x", "9/-6x\n54/36x^2"),
    ("x/x", "x/x\n0"),
    ("0/x+1", "x/x\n0"),
    ("1/x/x", "1/x^2\n-2x/x^4"),
    ("x/2/3", "x/6\n6/36"),
    ("1/(1/x)", "x\n1"),
    ("-(x+1)/(-x)", "(-x-1)/-x\n-1/x^2"),
    ("sin^2x+cos^2x", "sin^2x+cos^2x\n0"),
    ("x-x+5", "5\n0"),
    ("x^0sin^0xcos^0x", "1\n0"),
    ("(x+1)*(x-1)", "x^2-1\n2x"),
    ("1/x+1/x", "2x/x^2\n-2x^2/x^4"),
]
for expression, expected in cases:
    actual = subprocess.check_output([str(executable)], input=expression + "\n", text=True).strip()
    assert actual == expected, (expression, expected, actual)
print(f"Passed {len(cases)} regression cases.")
