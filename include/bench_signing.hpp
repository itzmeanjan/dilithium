#pragma once
#include "dilithium.hpp"
#include "utils.hpp"
#include <benchmark/benchmark.h>

// Benchmark Dilithium PQC suite implementation on CPU, using google-benchmark
namespace bench_dilithium {

// Benchmark Dilithium signing algorithm's performance with different parameters
// ( for three different NIST security levels )
template<const size_t k,
         const size_t l,
         const size_t d,
         const uint32_t η,
         const uint32_t γ1,
         const uint32_t γ2,
         const uint32_t τ,
         const uint32_t β,
         const size_t ω>
void
sign(benchmark::State& state)
{
  constexpr size_t slen = 32;
  constexpr size_t mlen = 32;
  constexpr size_t pklen = dilithium_utils::pubkey_length<k, d>();
  constexpr size_t sklen = dilithium_utils::seckey_length<k, l, η, d>();
  constexpr size_t siglen = dilithium_utils::signature_length<k, l, γ1, ω>();

  uint8_t* seed = static_cast<uint8_t*>(std::malloc(slen));
  uint8_t* pkey = static_cast<uint8_t*>(std::malloc(pklen));
  uint8_t* skey = static_cast<uint8_t*>(std::malloc(sklen));
  uint8_t* sig = static_cast<uint8_t*>(std::malloc(siglen));
  uint8_t* msg = static_cast<uint8_t*>(std::malloc(mlen));

  dilithium_utils::random_data<uint8_t>(seed, slen);
  dilithium_utils::random_data<uint8_t>(msg, mlen);

  dilithium::keygen<k, l, d, η>(seed, pkey, skey);

  for (auto _ : state) {
    dilithium::sign<k, l, d, η, γ1, γ2, τ, β, ω, mlen>(skey, msg, sig);

    benchmark::DoNotOptimize(skey);
    benchmark::DoNotOptimize(msg);
    benchmark::DoNotOptimize(sig);
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));

  bool flg = dilithium::verify<k, l, d, γ1, γ2, τ, β, ω, mlen>(pkey, msg, sig);

  std::free(seed);
  std::free(pkey);
  std::free(skey);
  std::free(sig);
  std::free(msg);

  assert(flg);
}

}
