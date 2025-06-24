# Compare & benchmark PWLF implementations

Testing and benchmarking implementation of Non Decreasing Continuous PieceWise Linear Functions. Such functions are crucial for Duration calculation when working in the context of Time-Dependent Routing Problems.

## Commands

##### Compile

Compile the project:

```shell
cd src/build/
cmake ..
make
```

Run:

```shell
python src/main.py
```

Use `compile.py` for automated compilation.

##### Run

`python src/main.py`: Classic run.
`valgrind --leak-check=full --show-reachable=yes python3 src/main.py &> valgrind_exec.log`: Run in debug mode and store valgrind outputs into a log file.
