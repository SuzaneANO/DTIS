#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

namespace dtis {

/*! \brief A reader visitor for the binary AIGER format. */
template<class Ntk>
class aiger_reader {
public:
  explicit aiger_reader(Ntk& ntk) : _ntk(ntk) {}

  ~aiger_reader() {
    uint32_t output_id{0};
    for (auto out : outputs) {
      auto const lit = std::get<0>(out);
      auto signal = signals[lit >> 1];
      if (lit & 1) {
        signal = _ntk.create_not(signal);
      }
      _ntk.create_po(signal);
    }
  }

  /*! \brief Callback method for parsed header. */
  virtual void on_header(uint64_t m, uint64_t num_inputs, uint64_t o, uint64_t a) const {
    _num_inputs = static_cast<uint32_t>(num_inputs);

    /* Constant */
    signals.push_back(_ntk.get_constant(false));

    /* Create primary inputs (pi) */
    for (auto i = 0u; i < num_inputs; ++i) {
      signals.push_back(_ntk.create_pi());
    }
  }

  /*! \brief Callback method for parsed input. */
  virtual void on_input(uint32_t pos, uint32_t lit) const {
    (void)pos;
    (void)lit;
  }

  /*! \brief Callback method for parsed output. */
  virtual void on_output(uint32_t index, uint32_t lit) const {
    assert(index == outputs.size());
    outputs.emplace_back(lit, "");
  }

  /*! \brief Callback method for parsed AND gate. */
  virtual void on_and(uint32_t index, uint32_t left_lit, uint32_t right_lit) const {
    (void)index;
    assert(signals.size() == index);

    auto left = signals[left_lit >> 1];
    if (left_lit & 1) {
      left = _ntk.create_not(left);
    }

    auto right = signals[right_lit >> 1];
    if (right_lit & 1) {
      right = _ntk.create_not(right);
    }

    signals.push_back(_ntk.create_and(left, right));
  }

  /*! \brief Callback method for parsed comment. */
  virtual void on_comment(const std::string& comment) const {
    (void)comment;
  }

private:
  Ntk& _ntk;
  mutable uint32_t _num_inputs{0};
  mutable std::vector<std::tuple<unsigned, std::string>> outputs;
  mutable std::vector<typename Ntk::signal_t> signals;
};

/*! \brief Reader function for binary AIGER format. */
template<class Ntk>
inline bool read_aiger(Ntk& ntk, std::istream& in) {
  aiger_reader<Ntk> reader(ntk);

  std::string header_line;
  getline(in, header_line);

  uint32_t _m, _i, _l, _o, _a;

  /* Parse header manually */
  std::istringstream ss(header_line);
  std::string prefix;
  if (!(ss >> prefix >> _m >> _i >> _l >> _o >> _a) || prefix != "aig") {
    return false;
  }
  reader.on_header(_m, _i, _o, _a);

  std::string line;

  /* Inputs */
  for (auto i = 0u; i < _i; ++i) {
    reader.on_input(i, 2u * (i + 1));
  }

  /* Outputs */
  for (auto i = 0u; i < _o; ++i) {
    getline(in, line);
    uint32_t lit = std::stoul(line);
    reader.on_output(i, lit);
  }

  const auto decode = [&]() {
    auto i = 0;
    auto res = 0;
    while (true) {
      auto c = in.get();
      res |= ((c & 0x7f) << (7 * i));
      if ((c & 0x80) == 0)
        break;
      ++i;
    }
    return res;
  };

  /* AND gates */
  for (auto i = _i + _l + 1; i < _i + _l + _a + 1; ++i) {
    const auto d1 = decode();
    const auto d2 = decode();
    const auto g = i << 1;
    reader.on_and(i, (g - d1), (g - d1 - d2));
  }

  return true;
}

/*! \brief Reader function for binary AIGER format. */
template<class Ntk>
inline bool read_aiger(Ntk& ntk, const std::string& file_path) {
  std::ifstream in(file_path, std::ifstream::binary);
  if (!in.is_open()) {
    return false;
  } else {
    auto const ret = read_aiger(ntk, in);
    in.close();
    return ret;
  }
}

} // namespace dtis

