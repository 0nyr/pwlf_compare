import sys
import os
import random

from utils.utils import get_a_parent_dir

PWLF_DIR = os.path.join(
    os.path.dirname(__file__),
    "build"
)
PLOT_DIR = os.path.join(
    get_a_parent_dir(os.path.dirname(__file__), 1),
    "plots"
)
RANDOM_SEED = 42
random.seed(RANDOM_SEED)
print(f"PWLF_DIR: {PWLF_DIR}")
print(f"PLOT_DIR: {PLOT_DIR}")

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


# Generate a plot of a 2D PWL function
import matplotlib.pyplot as plt
import numpy as np

def plot_raw_pwl_function(
    x_breakpoints: list,
    y_values : list,
    plot_subname: str
):
    fig, axs = plt.subplots(1, 1, figsize=(12, 5))
    fig.suptitle(f"PWLF plot: {plot_subname}", fontsize=16)

    # plot tau_ij
    ax = axs
    
    domain = [x_breakpoints[0], x_breakpoints[-1]]
    image = [min(y_values), max(y_values)]

    ax.plot(x_breakpoints, y_values, marker='o')
    ax.set_title(f"PWLF Function Plot: {plot_subname}, domain: {domain}, image: {image}")
    ax.set_xlabel(f"Domain axis (breakpoints)")
    ax.set_ylabel("Image axis (values)")
    ax.set_xlim(left=domain[0], right=domain[1])
    ax.set_ylim(bottom=image[0], top=image[1])
    ax.grid(True)

    plt.tight_layout()
    plt.subplots_adjust(top=0.88)

    # Save the plot
    
    plt.savefig(f"{PLOT_DIR}/pwl_function_plot-{plot_subname}.svg", format='svg')

    plt.show()

def plot_raw_pwl_functions(
        list_of_tuple_breakpoints_and_values: list[tuple[list, list]],
        plot_subname: str
):
    """
    Plot multiple PWL functions given a list of tuples containing breakpoints and values.
    """
    if list_of_tuple_breakpoints_and_values is None or len(list_of_tuple_breakpoints_and_values) <= 0:
        raise ValueError("list_of_tuple_breakpoints_and_values must be a non-empty list of tuples")

    fig, axs = plt.subplots(1, 1, figsize=(12, 5))
    fig.suptitle(f"PWLF plot: {plot_subname}", fontsize=16)

    global_domain = [float('inf'), float('-inf')]
    global_image = [float('inf'), float('-inf')]
    for i, (x_breakpoints, y_values) in enumerate(list_of_tuple_breakpoints_and_values):
        domain = [x_breakpoints[0], x_breakpoints[-1]]
        image = [min(y_values), max(y_values)]

        axs.plot(x_breakpoints, y_values, marker='o', label=f"Function {i+1}")

        # Update global domain and image
        global_domain[0] = min(global_domain[0], domain[0])
        global_domain[1] = max(global_domain[1], domain[1])
        global_image[0] = min(global_image[0], image[0])
        global_image[1] = max(global_image[1], image[1])

    axs.set_title(f"PWLF Functions Plot: {plot_subname}")
    axs.set_xlabel("Domain axis (breakpoints)")
    axs.set_ylabel("Image axis (values)")
    axs.set_xlim(left=global_domain[0], right=global_domain[1])
    axs.set_ylim(bottom=global_image[0], top=global_image[1])
    axs.grid(True)
    axs.legend()

    plt.tight_layout()
    plt.subplots_adjust(top=0.88)

    # Save the plot
    plt.savefig(f"{PLOT_DIR}/pwl_functions_plot-{plot_subname}.svg", format='svg')

    plt.show()


# Plot the PWL function
plot_raw_pwl_function(
    x_breakpoints=breakpoints,
    y_values=values,
    plot_subname="example"
)

def normalize_raw_pwl_function(
    x_breakpoints: list,
    y_values: list
):
    """
    Normalize a raw PWL function by ensuring no two consecutive pieces have the same slope,
    intercept, and share the end and beginning of their domains.
    Precondition: x_breakpoints are sorted in ascending order and y_values are the corresponding values.
    """
    if len(x_breakpoints) != len(y_values):
        raise ValueError("x_breakpoints and y_values must have the same length")

    normalized_x = []
    normalized_y = []

    # add the first point
    normalized_x.append(x_breakpoints[0])
    normalized_y.append(y_values[0])

    # Check that breakpoints are not too close to each other
    x_breakpoints_not_too_close = []
    y_associated_values = []
    x_breakpoints_not_too_close.append(x_breakpoints[0])
    y_associated_values.append(y_values[0])
    for i in range(1, len(x_breakpoints)):
        if pwlf_cpp.goc.epsilon_different(
            x_breakpoints[i], x_breakpoints_not_too_close[-1]
        ):
            x_breakpoints_not_too_close.append(x_breakpoints[i])
            y_associated_values.append(y_values[i])

    for i in range(1, len(x_breakpoints_not_too_close) - 1):
        x_prev = x_breakpoints_not_too_close[i-1]
        y_prev = y_associated_values[i-1]
        x_curr = x_breakpoints_not_too_close[i]
        y_curr = y_associated_values[i]          

        x_next = x_breakpoints_not_too_close[i+1]
        y_next = y_associated_values[i+1]

        slope1 = (y_curr - y_prev) / (x_curr - x_prev)
        slope2 = (y_next - y_curr) / (x_next - x_curr)

        if pwlf_cpp.goc.epsilon_different(slope1, slope2):
            # If slopes are different, add the current point
            normalized_x.append(x_curr)
            normalized_y.append(y_curr)
    
    # Add the last point
    if len(x_breakpoints_not_too_close) > 1:
        normalized_x.append(x_breakpoints_not_too_close[-1])
        normalized_y.append(y_associated_values[-1])

    return normalized_x, normalized_y

def generate_random_raw_pwl_function(
    num_breakpoints: int = 10,
    x_min: float = 10.0,
    x_max: float = 1000.0,
    y_min: float = 10.0,
    y_max: float = 1000.0
):
    """
    Generate a random PWL function with specified number of breakpoints and value ranges.
    WARN: Generated raw PWL function is normalized: A function is normalized iif no two 
    consecutive pieces have the same slope, intercept, and share the end and beginning 
    of their domains.
    """
    x_breakpoints = sorted(random.sample(
        [random.uniform(x_min, x_max) for _ in range(num_breakpoints)],
        num_breakpoints
    ))
    y_values = [random.uniform(y_min, y_max) for _ in range(num_breakpoints)]
    
    return normalize_raw_pwl_function(
        x_breakpoints=x_breakpoints,
        y_values=y_values
    )

chain_length = 10
goc_functions_for_chain = []
nyr_functions_for_chain = []

for i in range(chain_length):
    x_breakpoints, y_values = generate_random_raw_pwl_function(
        num_breakpoints=random.randint(3, 10)
    )
    plot_subname = f"random-{i}-breakpoints-{len(x_breakpoints)}"
    plot_raw_pwl_function(
        x_breakpoints=x_breakpoints,
        y_values=y_values,
        plot_subname=plot_subname
    )

    goc_functions_for_chain.append(
        pwlf_cpp.goc.PWLFunction(x_breakpoints, y_values)
    )
    nyr_functions_for_chain.append(
        pwlf_cpp.nyr.CPWLF(x_breakpoints, y_values)
    )

# Plot all the goc functions
plot_raw_pwl_functions(
    list_of_tuple_breakpoints_and_values=[
        goc_function.copy_breakpoints_and_values() for goc_function in goc_functions_for_chain
    ],
    plot_subname="goc_chain"
)

# Chain composition
goc_composed_function = goc_functions_for_chain[0]
nyr_composed_function = nyr_functions_for_chain[0]
for i in range(1, chain_length):
    goc_composed_function = goc_composed_function.compose(goc_functions_for_chain[i])
    nyr_composed_function = nyr_composed_function.compose(nyr_functions_for_chain[i])

composed_raw_pwlfs = []
composed_raw_pwlfs.append(
    goc_composed_function.copy_breakpoints_and_values()
)
composed_raw_pwlfs.append(
    nyr_composed_function.copy_breakpoints_and_values()
)


