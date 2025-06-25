#!/usr/bin/env python3
import os
import sys
import argparse
import subprocess
import datetime
import multiprocessing

# where to put out-of-tree builds
BUILD_ROOT_DIR = "build"
# where your CMakeLists.txt is
CMAKELISTS_DIR = os.path.join(os.path.dirname(__file__), "src")
# absolute paths
PROJECT_ROOT = os.path.abspath(os.path.dirname(__file__))
OBJ_ROOT     = os.path.join(PROJECT_ROOT, BUILD_ROOT_DIR)


def clean_build_dir(build_type: str):
    """
    rm -rf build/<build_type>
    """
    path = os.path.join(OBJ_ROOT, build_type)
    if os.path.isdir(path):
        print(f"[clean] removing {path}")
        if not path.startswith(PROJECT_ROOT):
            raise RuntimeError(f"Refusing to remove outside project: {path}")
        subprocess.check_call(["rm", "-rf", path])


def configure_and_build(build_type: str, clean: bool, just_compile: bool):
    build_dir = os.path.join(OBJ_ROOT, build_type)
    os.makedirs(build_dir, exist_ok=True)

    if clean:
        clean_build_dir(build_type)
        os.makedirs(build_dir, exist_ok=True)

    # prepare 
    cmake_args = [
        "cmake",
        CMAKELISTS_DIR,
        f"-DCMAKE_BUILD_TYPE={build_type.capitalize()}"
    ]
    nprocs = multiprocessing.cpu_count() or 1
    make_args  = ["make", f"-j{nprocs}"]

    print(f"\n>>> [{build_type}] CMake: {' '.join(cmake_args)}")
    print(f">>> [{build_type}] Make:  {' '.join(make_args)}")
    t0 = datetime.datetime.now()

    # run cmake
    subprocess.check_call(cmake_args, cwd=build_dir)
    if not just_compile:
        subprocess.check_call(make_args, cwd=build_dir)

    elapsed = (datetime.datetime.now() - t0).total_seconds()
    print(f"[{build_type}] done in {elapsed:.3f}s")


def main():
    p = argparse.ArgumentParser(description="Configure & build the C++ solver")
    p.add_argument(
        "--mode","-m",
        choices=("debug","release","fastdebug","all"),
        default="all",
        help="Which configuration to build"
    )
    p.add_argument(
        "--clean","-c",
        action="store_true",
        help="rm -rf build/<mode> before building"
    )
    p.add_argument(
        "--just-compile","-j",
        action="store_true",
        help="Run cmake only, skip make"
    )
    args = p.parse_args()

    targets = [args.mode] if args.mode in ("debug", "release", "fastdebug") else ["debug", "release", "fastdebug"]
    for mode in targets:
        configure_and_build(mode, clean=args.clean, just_compile=args.just_compile)


if __name__ == "__main__":
    main()
