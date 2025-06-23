import sys
import os
import random

from utils.utils import get_a_parent_dir

PROJECT_ROOT = get_a_parent_dir(os.path.dirname(__file__), 1)

PWLF_BUILD_TYPE = "release" # "debug" or "release"
PWLF_DIR = os.path.join(
    PROJECT_ROOT,
    f"build/{PWLF_BUILD_TYPE}"
)
PLOT_DIR = os.path.join(
    PROJECT_ROOT,
    "plots"
)
RANDOM_SEED = 42
random.seed(RANDOM_SEED)
print(f"PWLF_DIR: {PWLF_DIR}")
print(f"PLOT_DIR: {PLOT_DIR}")

sys.path.append(PWLF_DIR)  # or wherever the .so is
import pwlf_cpp

pwlf_cpp.nyr.test_interval_vector_intersects()
pwlf_cpp.nyr.test_interval_vector_includes()

# # Create intervals
# interval = pwlf_cpp.goc.Interval(0.0, 10.0)

# # Create PWL functions
# breakpoints = [0.0, 1.0, 2.0, 3.0]
# values = [0.0, 2.0, 1.0, 3.0]
# pwl = pwlf_cpp.goc.PWLFunction(breakpoints, values)

# # Use CPWLF from nyr namespace
# cpwlf = pwlf_cpp.nyr.CPWLF(breakpoints, values)

# print("Result of PWL evaluation at 1.5:", pwl(1.5))
# print("Result of CPWLF evaluation at 1.5:", cpwlf(1.5))


# Generate a plot of a 2D PWL function
import matplotlib.pyplot as plt
import pandas as pd
import sys
import time

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

def plot_goc_pwl_function(
    pwl_function: pwlf_cpp.goc.PWLFunction,
    plot_subname: str
):
    """
    This function is represented by a list of pieces.
    So we need to plot each piece separately.
    """
    if pwl_function is None:
        raise ValueError("pwl_function must not be None")
    
    fig, axs = plt.subplots(1, 1, figsize=(12, 5))
    fig.suptitle(f"PWLFunction plot: {plot_subname}", fontsize=16)

    # plot tau_ij
    ax = axs
    
    domain = [pwl_function.domain().left, pwl_function.domain().right]
    image = [pwl_function.image().left, pwl_function.image().right]

    for i in range(pwl_function.piece_count()):
        piece = pwl_function.piece(i)
        x_breakpoints = [piece.domain.left, piece.domain.right]
        y_values = [
            piece(x) for x in x_breakpoints
        ]
        ax.plot(x_breakpoints, y_values, marker='o', label=f"Piece n°{i}")
    
    ax.set_title(f"PWLFunction Plot {plot_subname}. domain: {domain}, image: {image}")
    ax.set_xlabel(f"Domain axis (breakpoints)")
    ax.set_ylabel("Image axis (values)")
    ax.set_xlim(left=domain[0], right=domain[1])
    ax.set_ylim(bottom=image[0], top=image[1])
    ax.grid(True)

    plt.tight_layout()
    plt.subplots_adjust(top=0.88)

    # Save the plot
    
    plt.savefig(f"{PLOT_DIR}/PWLFunction_plot-{plot_subname}.svg", format='svg')

    plt.show()

def plot_nyr_cpwlf_function(
    cpwlf_function: pwlf_cpp.nyr.CPWLF,
    plot_subname: str
):
    """
    This function is represented by a list of breakpoints and values.
    It is continuous over the whole domain.
    """
    if cpwlf_function is None:
        raise ValueError("cpwlf_function must not be None")
    
    fig, axs = plt.subplots(1, 1, figsize=(12, 5))
    fig.suptitle(f"CPWLF plot: {plot_subname}", fontsize=16)

    # plot tau_ij
    ax = axs
    
    domain = [cpwlf_function.get_domain().left, cpwlf_function.get_domain().right]
    image = [cpwlf_function.get_image().left, cpwlf_function.get_image().right]

    x_breakpoints = cpwlf_function.get_breakpoints()
    y_values = cpwlf_function.get_values()

    ax.plot(x_breakpoints, y_values, marker='o')
    
    ax.set_title(f"CPWLF Function Plot: {plot_subname}, domain: {domain}, image: {image}")
    ax.set_xlabel(f"Domain axis (breakpoints)")
    ax.set_ylabel("Image axis (values)")
    ax.set_xlim(left=domain[0], right=domain[1])
    ax.set_ylim(bottom=image[0], top=image[1])
    ax.grid(True)

    plt.tight_layout()
    plt.subplots_adjust(top=0.88)

    # Save the plot
    plt.savefig(f"{PLOT_DIR}/CPWLF_plot-{plot_subname}.svg", format='svg')

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
# plot_raw_pwl_function(
#     x_breakpoints=breakpoints,
#     y_values=values,
#     plot_subname="example"
# )

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

def make_bounded_slope(
    xs: list,
    ys: list,
    min_slope: float = -1.0,
    max_slope: float = 1e6
):
    """
    Given a 2D PWL function defined by x_breakpoints and y_values,
    ensure that the slope between each pair of consecutive points is bounded.
    First and last points are not modified. 
    
    For each segment i>0, slope_i = (y_i' - y_{i-1}')/(x_i - x_{i-1})
    is clamped to [min_slope, max_slope].

    We build left-to-right. Whenever the "raw" (y_i - y_{i-1}')/(dx) is out of bounds,
    we set y_i' = y_{i-1}' + clamp(slope_raw) * dx.
    """

    if len(xs) != len(ys):
        raise ValueError("xs and ys must have the same length")

    new_xs: list[float] = []
    new_ys: list[float] = []
    # first point is always kept
    new_xs.append(xs[0])  
    new_ys.append(ys[0])

    for x_i, y_i_orig in zip(xs[1:], ys[1:]):
        x_prev, y_prev = new_xs[-1], new_ys[-1]
        dx = x_i - x_prev
        if dx <= 0:
            raise ValueError(f"Non-increasing breakpoint: {x_prev} → {x_i}")

        raw_slope = (y_i_orig - y_prev) / dx
        # clamp the slope into [min_slope, max_slope]
        bounded_slope = min(max(raw_slope, min_slope), max_slope)

        # WARN: Due to rounding imprecision, when the slope is almost 0.0
        # prefer to copy y_prev instead of computing y_i = y_prev + 0.0 * dx
        if pwlf_cpp.goc.epsilon_equal(bounded_slope, 0.0):
            y_i = y_prev
        # if we had to clamp, recompute y_i'; else keep original y_i_orig
        elif bounded_slope != raw_slope:
            y_i = y_prev + bounded_slope * dx
        else:
            y_i = y_i_orig

        new_xs.append(x_i)
        new_ys.append(y_i)

    return normalize_raw_pwl_function(new_xs, new_ys)

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
    if num_breakpoints < 2:
        raise ValueError("num_breakpoints must be at least 2 for a non-decreasing PWL function")

    x_breakpoints = sorted(random.sample(
        [random.uniform(x_min, x_max) for _ in range(num_breakpoints)],
        num_breakpoints
    ))
    y_values = [random.uniform(y_min, y_max) for _ in range(num_breakpoints)]
    
    return normalize_raw_pwl_function(
        x_breakpoints=x_breakpoints,
        y_values=y_values
    )

def make_non_decreasing_pwl_function(
    xs: list[float],
    ys: list[float],
):
    """
    Given a 2D PWL function defined by xs and ys, ensure that the function is non-decreasing.
    Only the first point is not modified.
    We compute f'(x) = f(x) + x
    """

    # Bound the slope to ensure non-decreasing property
    bounded_slope_xs, bounded_slope_ys = make_bounded_slope(
        xs=xs,
        ys=ys,
    )
    # Ensure the function is non-decreasing
    for i in range(1, len(bounded_slope_ys)):
        x = bounded_slope_xs[i]
        y_new = x + bounded_slope_ys[i]

        # WARN: due to rounding imprecision, when the 2 consecutive y values are almost equal,
        # prefer to copy the previous y value instead of computing it.
        y_prev = bounded_slope_ys[i-1]
        if pwlf_cpp.goc.epsilon_equal(y_new, y_prev):
            bounded_slope_ys[i] = y_prev
        else:
            bounded_slope_ys[i] = y_new

    return normalize_raw_pwl_function(
        x_breakpoints=bounded_slope_xs,
        y_values=bounded_slope_ys
    )

def generate_non_decreasing_raw_pwl_function(
    num_breakpoints: int = 10,
    x_min: float = 0.0,
    x_max: float = 100.0,
    y_min: float = 1.0,
    y_max: float = 10.0
):
    """
    Generate a non-decreasing PWL function with specified number of breakpoints and value ranges.
    The function is normalized.
    """
    xs, ys = generate_random_raw_pwl_function(
        num_breakpoints=num_breakpoints,
        x_min=x_min,
        x_max=x_max,
        y_min=y_min,
        y_max=y_max
    )
    return make_non_decreasing_pwl_function(
        xs=xs,
        ys=ys
    )

def assert_non_decreasing_pwl_function(
    xs: list[float],
    ys: list[float],
    message: str = ""
):
    """
    Assert that the given PWL function defined by xs and ys is non-decreasing.
    """
    if len(xs) != len(ys):
        raise ValueError(f"{message}: xs and ys must have the same length")

    for i in range(1, len(xs)):
        # if xs[i] < xs[i-1]:
        if pwlf_cpp.goc.epsilon_smaller(xs[i], xs[i-1]):
            raise ValueError(f"{message}: Non-increasing breakpoint: {xs[i-1]} → {xs[i]}")
        # if ys[i] < ys[i-1]:
        if pwlf_cpp.goc.epsilon_smaller(ys[i], ys[i-1]):
            raise ValueError(f"{message}: Non-decreasing violation at index {i}: {ys[i-1]} → {ys[i]}")

def get_size_in_bytes(obj):
    """
    Get the memory size of a C++-backed object in bytes.
    This assumes the C++ binding exposes a method 
    called `memory_footprint_bytes()`.
    """
    if hasattr(obj, "memory_footprint_bytes"):
        return obj.memory_footprint_bytes()
    else:
        # Fallback: Python object size (not accurate for C++ objects)
        return sys.getsizeof(obj)

def test_bounded_slope_pwl_generation():
    functions: list[tuple[list, list]] = [
        (generate_random_raw_pwl_function(
            num_breakpoints=40,
            x_min=0.0,
            x_max=100.0,
            y_min=1.0,
            y_max=10.0
        ))
    ]
    functions.append(
        make_bounded_slope(
            xs=functions[0][0],
            ys=functions[0][1]
        )
    )
    functions.append(
        make_non_decreasing_pwl_function(
            xs=functions[0][0],
            ys=functions[0][1]
        )
    )   
            
    plot_raw_pwl_functions(
        list_of_tuple_breakpoints_and_values=functions,
        plot_subname="slope_bounding_example"
    )

def benchmark_pwl_function_implementation(
        composition_chain_lengths: list[int],
        pwlf_nb_breakpoints: list[int],
        check_functions: bool = False,
        non_decreasing: bool = False,
        num_breakpoints: int = 6
    ):
    """
    Benchmark the PWL function implementation by creating a chain
    of PWL functions and measuring the time and memory usage.
    """
    df = pd.DataFrame()

    for num_breakpoints in pwlf_nb_breakpoints:

        # 1. Generate a chain of PWL functions
        max_chain_length = max(composition_chain_lengths)
        goc_functions_for_chain = []
        nyr_functions_for_chain = []

        for i in range(max_chain_length):
            if non_decreasing:
                x_breakpoints, y_values = generate_non_decreasing_raw_pwl_function(
                    num_breakpoints=num_breakpoints,
                    x_min=0.0,
                    x_max=100000.0,
                    y_min=1.0,
                    y_max=100.0
                )
                assert_non_decreasing_pwl_function(
                    xs=x_breakpoints,
                    ys=y_values,
                    message=f"While generating non-decreasing PWL function no. {i+1}"
                )
            else:
                x_breakpoints, y_values = generate_random_raw_pwl_function(
                num_breakpoints=num_breakpoints,
            )
            # plot_subname = f"random-{i}-breakpoints-{len(x_breakpoints)}"
            # plot_raw_pwl_function(
            #     x_breakpoints=x_breakpoints,
            #     y_values=y_values,
            #     plot_subname=plot_subname
            # )

            goc_functions_for_chain.append(
                pwlf_cpp.goc.PWLFunction(x_breakpoints, y_values)
            )
            nyr_functions_for_chain.append(
                pwlf_cpp.nyr.NDCPWLF(x_breakpoints, y_values)
            )

        # Check all the functions
        if check_functions:
            print("Checking invariants and normalization for all functions in the chain:")
            for i in range(max_chain_length):
                goc_inv = goc_functions_for_chain[i].check_invariant()
                print(f"goc_functions_for_chain[{i}].check_invariant(): {goc_inv}")
                goc_norm = goc_functions_for_chain[i].check_normalization()
                print(f"goc_functions_for_chain[{i}].check_normalization(): {goc_norm}")
                nyr_inv = nyr_functions_for_chain[i].check_invariant()
                print(f"nyr_functions_for_chain[{i}].check_invariant(): {nyr_inv}")
                nyr_norm = nyr_functions_for_chain[i].check_normalization()
                print(f"nyr_functions_for_chain[{i}].check_normalization(): {nyr_norm}")

                # Print functions
                print(f"goc_functions_for_chain[{i}]: {goc_functions_for_chain[i]}")
                print(f"nyr_functions_for_chain[{i}]: {nyr_functions_for_chain[i]}")
            

        # Plot all the goc functions
        # plot_raw_pwl_functions(
        #     list_of_tuple_breakpoints_and_values=[
        #         goc_function.copy_breakpoints_and_values() for goc_function in goc_functions_for_chain
        #     ],
        #     plot_subname="goc_chain"
        # )

        # 2. Perform compositions
        for chain_length in composition_chain_lengths:
            # Chain composition for goc
            goc_composed_function = goc_functions_for_chain[0]
            start_goc = time.time()
            for i in range(1, chain_length):
                goc_composed_function = goc_composed_function.compose(goc_functions_for_chain[i])
            goc_compose_time = time.time() - start_goc
            if chain_length < 100 and num_breakpoints < 100:
                print("GOC composed function:", goc_composed_function)

            # Chain composition for nyr
            nyr_composed_function = nyr_functions_for_chain[0]
            start_nyr = time.time()
            for i in range(1, chain_length):
                nyr_composed_function = nyr_composed_function.compose(nyr_functions_for_chain[i])
            nyr_compose_time = time.time() - start_nyr
            if chain_length < 100 and num_breakpoints < 100:
                print("Nyr composed function:", nyr_composed_function)

            if non_decreasing:
                try:
                    assert_non_decreasing_pwl_function(
                        xs=nyr_composed_function.get_breakpoints(),
                        ys=nyr_composed_function.get_values(),
                        message=f"While checking non-decreasing property of nyr_composed_function after {chain_length} compositions"
                    )
                    xs, ys = goc_composed_function.copy_breakpoints_and_values()
                    assert_non_decreasing_pwl_function(
                        xs=xs,
                        ys=ys,
                        message=f"While checking non-decreasing property of goc_composed_function after {chain_length} compositions"
                    )
                except ValueError as e:
                    print(f"Non-decreasing assertion failed: {e}")

            # if num_breakpoints == 6 and chain_length == 5:
            # if num_breakpoints == 1001 and chain_length == 10:
                # plot_goc_pwl_function(
                #     pwl_function=goc_composed_function,
                #     plot_subname="goc_composed"
                # )
                # plot_nyr_cpwlf_function(
                #     cpwlf_function=nyr_composed_function,
                #     plot_subname="nyr_composed"
                # )

            #print(goc_composed_function.copy_breakpoints_and_values())

            # Gather stats
            stats = {
                "Start nb pieces": [
                    goc_functions_for_chain[0].piece_count(),
                    nyr_functions_for_chain[0].nb_pieces()
                ],
                "Chain Length": chain_length,
                "Function": ["goc_composed", "nyr_composed"],
                "Num Pieces": [
                    goc_composed_function.piece_count(),
                    nyr_composed_function.nb_pieces()
                ],
                "Area": [
                    goc_composed_function.compute_area(),
                    nyr_composed_function.compute_area()
                ],
                "Memory Used (Bytes)": [
                    get_size_in_bytes(goc_composed_function),
                    get_size_in_bytes(nyr_composed_function)
                ],
                "Compose Time (s)": [
                    goc_compose_time,
                    nyr_compose_time
                ]
            }

            df_current_stats = pd.DataFrame(stats)
            print(df_current_stats.to_markdown(index=False))
            df = pd.concat(
                [df, pd.DataFrame(stats)],
                ignore_index=True
            )

    # 3. Print the full results
    print(f"\nFull benchmark results: [build type: {PWLF_BUILD_TYPE}]")
    print(df.to_markdown(index=False))

    # 5. Print total time and memory usage for each implementation
    print("\nPWLF type:", "non-decreasing" if non_decreasing else "raw")
    total_time_goc = df[df["Function"] == "goc_composed"]["Compose Time (s)"].sum()
    total_time_nyr = df[df["Function"] == "nyr_composed"]["Compose Time (s)"].sum()
    total_memory_goc = df[df["Function"] == "goc_composed"]["Memory Used (Bytes)"].sum()
    total_memory_nyr = df[df["Function"] == "nyr_composed"]["Memory Used (Bytes)"].sum()
    print("Total time and memory usage:")
    print(f"Total Time GOC: {total_time_goc} s")
    print(f"Total Time NYR: {total_time_nyr} s")
    print(f"Total Memory GOC: {total_memory_goc} Bytes")
    print(f"Total Memory NYR: {total_memory_nyr} Bytes")

    # Print fastest, and most memory-efficient implementation
    if total_time_goc < total_time_nyr:
        print("GOC implementation is faster.")
    elif total_time_nyr < total_time_goc:
        print("NYR implementation is faster.")
    else:
        print("Both implementations have the same execution time.")

    if total_memory_goc < total_memory_nyr:
        print("GOC implementation is more memory-efficient.")
    elif total_memory_nyr < total_memory_goc:
        print("NYR implementation is more memory-efficient.")
    else:
        print("Both implementations use the same amount of memory.")


benchmark_pwl_function_implementation(
    composition_chain_lengths=[2, 5, 10, 100, 500],
    pwlf_nb_breakpoints=[6, 12, 100, 1000],
    check_functions=False,
    non_decreasing=True
)