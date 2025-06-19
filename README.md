# Compare & benchmark PWLF implementations

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

`valgrind --leak-check=full --show-reachable=yes python3 src/main.py`: Run in debug mode.
