#include "bench/bench_dilithium.hpp"

// register for benchmarking Dilithium Key Generation, Signing & Verification
using namespace bench_dilithium;

BENCHMARK(keygen<4, 4, 13, 2>)->UseManualTime();
BENCHMARK(sign<4, 4, 13, 2, 131072, 95232, 39, 78, 80>)
  ->Arg(32)
  ->UseManualTime();
BENCHMARK(verify<4, 4, 13, 2, 131072, 95232, 39, 78, 80>)
  ->Arg(32)
  ->UseManualTime();
BENCHMARK(keygen<6, 5, 13, 4>)->UseManualTime();
BENCHMARK(sign<6, 5, 13, 4, 524288, 261888, 49, 196, 55>)
  ->Arg(32)
  ->UseManualTime();
BENCHMARK(verify<6, 5, 13, 4, 524288, 261888, 49, 196, 55>)
  ->Arg(32)
  ->UseManualTime();
BENCHMARK(keygen<8, 7, 13, 2>)->UseManualTime();
BENCHMARK(sign<8, 7, 13, 2, 524288, 261888, 60, 120, 75>)
  ->Arg(32)
  ->UseManualTime();
BENCHMARK(verify<8, 7, 13, 2, 524288, 261888, 60, 120, 75>)
  ->Arg(32)
  ->UseManualTime();

// benchmark runner main routine
BENCHMARK_MAIN();
