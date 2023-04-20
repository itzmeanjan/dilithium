#include "bench/bench_dilithium.hpp"

auto compute_min = [](const std::vector<double>& v) -> double {
  return *std::min_element(v.begin(), v.end());
};

auto compute_max = [](const std::vector<double>& v) -> double {
  return *std::max_element(v.begin(), v.end());
};

using namespace bench_dilithium;

// register for benchmarking Dilithium Key Generation, Signing & Verification

BENCHMARK(dilithium2_keygen)
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(dilithium2_sign)
  ->Arg(32)
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);
BENCHMARK(dilithium2_verify)
  ->Arg(32)
  ->ComputeStatistics("min", compute_min)
  ->ComputeStatistics("max", compute_max);

BENCHMARK(dilithium3_keygen)->UseManualTime();
BENCHMARK(dilithium3_sign)->Arg(32)->UseManualTime();
BENCHMARK(dilithium3_verify)->Arg(32)->UseManualTime();

BENCHMARK(dilithium5_keygen)->UseManualTime();
BENCHMARK(dilithium5_sign)->Arg(32)->UseManualTime();
BENCHMARK(dilithium5_verify)->Arg(32)->UseManualTime();

// benchmark runner main routine
BENCHMARK_MAIN();
