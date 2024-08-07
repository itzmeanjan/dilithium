#include "ml_dsa/ml_dsa_44.hpp"
#include <cstdio>

#define DUDECT_IMPLEMENTATION
#define DUDECT_VISIBLITY_STATIC
#include "dudect.h"

constexpr size_t SEED_LEN = 32; // Byte length of seed(s)

uint8_t
do_one_computation(uint8_t* const data)
{
  constexpr uint32_t α = ml_dsa_44::γ2 << 1;
  constexpr uint32_t m = (ml_dsa_field::Q - 1u) / α;
  constexpr size_t w1bw = std::bit_width(m - 1u);

  constexpr size_t doff0 = 0;
  constexpr size_t doff1 = doff0 + 2 * SEED_LEN;

  std::array<ml_dsa_field::zq_t, ml_dsa_44::l * ml_dsa_ntt::N> vec{};
  std::array<ml_dsa_field::zq_t, vec.size()> vec_high{};
  std::array<ml_dsa_field::zq_t, vec.size()> vec_low{};
  std::array<ml_dsa_field::zq_t, vec_high.size()> vec_hint{};
  std::array<uint8_t, (vec_high.size() * w1bw) / 8> encoded{};
  std::array<ml_dsa_field::zq_t, vec_high.size()> decoded{};
  std::array<uint8_t, ml_dsa_44::ω + ml_dsa_44::l> encoded_hints{};
  std::array<ml_dsa_field::zq_t, vec.size()> decoded_hints{};

  auto seed = std::span<const uint8_t, 2 * SEED_LEN>(data + doff0, doff1 - doff0);
  const uint16_t kappa = (static_cast<uint16_t>(data[doff1 + 1]) << 8) | (static_cast<uint16_t>(data[doff1 + 0]) << 0);

  uint8_t ret_val = 0;

  ml_dsa_sampling::expand_mask<ml_dsa_44::γ1, ml_dsa_44::l>(seed, kappa, vec);
  ret_val ^= static_cast<uint8_t>(vec[0].raw() ^ vec[vec.size() - 1].raw());

  ml_dsa_polyvec::ntt<ml_dsa_44::l>(vec);
  ret_val ^= static_cast<uint8_t>(vec[0].raw() ^ vec[vec.size() - 1].raw());

  ml_dsa_polyvec::intt<ml_dsa_44::l>(vec);
  ret_val ^= static_cast<uint8_t>(vec[0].raw() ^ vec[vec.size() - 1].raw());

  ml_dsa_polyvec::highbits<ml_dsa_44::l, α>(vec, vec_high);
  ret_val ^= static_cast<uint8_t>(vec_high[0].raw() ^ vec_high[vec_high.size() - 1].raw());

  ml_dsa_polyvec::lowbits<ml_dsa_44::l, α>(vec, vec_low);
  ret_val ^= static_cast<uint8_t>(vec_low[0].raw() ^ vec_low[vec_low.size() - 1].raw());

  ml_dsa_polyvec::encode<ml_dsa_44::l, w1bw>(vec_high, encoded);
  ret_val ^= encoded[0] ^ encoded[encoded.size() - 1];

  ml_dsa_polyvec::decode<ml_dsa_44::l, w1bw>(encoded, decoded);
  ret_val ^= static_cast<uint8_t>(decoded[0].raw() ^ decoded[decoded.size() - 1].raw());

  const auto z_norm = ml_dsa_polyvec::infinity_norm<ml_dsa_44::l>(vec);
  ret_val ^= static_cast<uint8_t>(z_norm.raw());

  ml_dsa_polyvec::make_hint<ml_dsa_44::l, α>(vec, vec_high, vec_hint);
  ret_val ^= static_cast<uint8_t>(vec_high[0].raw() ^ vec_hint[vec_hint.size() - 1].raw());

  const auto count_1 = ml_dsa_polyvec::count_1s<ml_dsa_44::l>(vec_hint);
  ret_val ^= static_cast<uint8_t>(count_1);

  ml_dsa_bit_packing::encode_hint_bits<ml_dsa_44::l, ml_dsa_44::ω>(vec_hint, encoded_hints);
  ret_val ^= encoded_hints[0] ^ encoded_hints[encoded_hints.size() - 1];

  ml_dsa_bit_packing::decode_hint_bits<ml_dsa_44::l, ml_dsa_44::ω>(encoded_hints, decoded_hints);
  ret_val ^= static_cast<uint8_t>(decoded_hints[0].raw() ^ decoded_hints[decoded_hints.size() - 1].raw());

  return ret_val;
}

void
prepare_inputs(dudect_config_t* const c, uint8_t* const input_data, uint8_t* const classes)
{
  randombytes(input_data, c->number_measurements * c->chunk_size);

  for (size_t i = 0; i < c->number_measurements; i++) {
    classes[i] = randombit();
    if (classes[i] == 0) {
      std::memset(input_data + i * c->chunk_size, 0x00, c->chunk_size);
    }
  }
}

dudect_state_t
test_ml_dsa_44()
{
  constexpr size_t chunk_size = 2 * SEED_LEN + 2 + SEED_LEN;
  constexpr size_t number_measurements = 1e5;

  dudect_config_t config = {
    chunk_size,
    number_measurements,
  };
  dudect_ctx_t ctx;
  dudect_init(&ctx, &config);

  dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
  while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {
    state = dudect_main(&ctx);
  }

  dudect_free(&ctx);

  printf("Detected timing leakage in \"%s\", defined in file \"%s\"\n", __func__, __FILE_NAME__);
  return state;
}

int
main()
{
  if (test_ml_dsa_44() != DUDECT_NO_LEAKAGE_EVIDENCE_YET) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
