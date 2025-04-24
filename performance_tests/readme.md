# Create FlameGraph
- install perf
`sudo apt install linux-tools-generic`
- build the project
- run script:
```
perf record -g ./cdma_decoder ../cdma_decoder/signal5.txt
perf script > out.perf
git clone https://github.com/brendangregg/FlameGraph.git
cd FlameGraphsudo apt
./stackcollapse-perf.pl ../out.perf > out.folded
./flamegraph.pl out.folded > perf_flamegraph.svg
```
- view diagram in browser

# Build the project
```
cmake -S . -B build -DCMAKE_CXX_FLAGS="-O3" # for Debug Symbols: -g, additional optimizing flags: -march=native -mavx -msse4.2
cmake --build build --config Release
```
# Run the program
```
./build/cdma_decoder cdma_decoder/signal5.txt
```