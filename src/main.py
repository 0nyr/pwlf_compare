import sys
import os

PWLF_DIR = os.path.join(
    os.path.dirname(__file__),
    "build/"
)

sys.path.append(PWLF_DIR)  # or wherever the .so is
import pwlf_cpp


# Create intervals
interval = pwlf_cpp.goc.Interval(0.0, 10.0)

# Create PWL functions
breakpoints = [0.0, 1.0, 2.0, 3.0]
values = [0.0, 2.0, 1.0, 3.0]
pwl = pwlf_cpp.goc.PWLFunction(breakpoints, values)

# Use CPWLF from nyr namespace
cpwlf = pwlf_cpp.nyr.CPWLF(breakpoints, values)

print("Result of PWL evaluation at 1.5:", pwl(1.5))
print("Result of CPWLF evaluation at 1.5:", cpwlf(1.5))

