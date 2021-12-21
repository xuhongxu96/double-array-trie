#include "datrie_builder.h"
#include "serializers/compact_serializer.h"
#include <algorithm>
#include <boost/ut.hpp>
#include <string_view>
#include <testcases.h>
#include <vector>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  "test _bit_scan_forward"_test = [] {
    unsigned long index;
    expect(details::_bit_scan_forward(&index, 0b0100110) == 1);
    expect(index == 1);

    expect(details::_bit_scan_forward(&index, 0b0100100) == 1);
    expect(index == 2);

    expect(details::_bit_scan_forward(&index, 0b0100000) == 1);
    expect(index == 5);

    expect(details::_bit_scan_forward(&index, 0b0) == 0);
  };

  "test _bit_scan_reverse"_test = [] {
    unsigned long index;
    expect(details::_bit_scan_reverse(&index, 0b0100110) == 1);
    expect(index == 5);

    expect(details::_bit_scan_reverse(&index, 0b0000110) == 1);
    expect(index == 2);

    expect(details::_bit_scan_reverse(&index, 0b0000010) == 1);
    expect(index == 1);

    expect(details::_bit_scan_reverse(&index, 0b0) == 0);
  };

  "test TransSet"_test = [] {
    using namespace std::string_view_literals;

    auto test = [](std::string_view input_trans, std::string_view expected) {
      TransSet trans;
      for (char ch : input_trans) {
        trans.add(ch);
      }

      std::string res;
      for (auto it = trans.begin(); !it.end(); ++it) {
        res += static_cast<char>(it.trans());
      }

      expect(res == expected);

      if (!expected.empty()) {
        expect(trans.front() == expected.front());
        expect(trans.back() == expected.back());
      }
    };

    test("", "");
    test("adivzx", "adivxz");
    test("a", "a");
    test("z", "z");
    test("za", "az");
  };

  "test build"_test = [] {
    DoubleArrayTrieBuilder builder;
    builder.add("hello", 0);
    builder.add("hi", 1);
    builder.end_build();

    auto it = builder.traverse("h");
    expect(it.matched());
    expect(builder.has_value_at(it.state()) == false);

    it = builder.traverse("hello");
    expect(it.matched());
    expect(builder.value_at(it.state()) == 0);

    it = builder.traverse("hi");
    expect(it.matched());
    expect(builder.value_at(it.state()) == 1);
  };

  add_common_tests<DoubleArrayTrieBuilder<>, CompactSerializer>();

  return 0;
}
