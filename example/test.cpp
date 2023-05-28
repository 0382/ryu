#include "ryu/ryu.h"
#include <charconv>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <cassert>

int main(int argc, char const *argv[]) {
  double x = 0.0;
  double v[] = {1.0, 0.1, -0.0, -1.0, 3.14159, 1.618e10, -3.142857142857142857, x / x, 1.0 / x, -1.0 / x};
  constexpr std::size_t width = 25;
  std::cout << std::setw(width) << "d2s";
  std::cout << std::setw(width) << "d2fixed";
  std::cout << std::setw(width) << "d2exp";
  std::cout << std::setw(width) << "c++ std::cout";
  std::cout << std::setw(width) << "write_shortest";
  std::cout << std::setw(width) << "to_chars/fixed";
  std::cout << std::setw(width) << "to_chars/scientific";
  std::cout << std::setw(width) << "to_chars/general\n";
  for (auto f : v) {
    // std::size_t word;
    // std::memcpy(&word, &f, sizeof(double));
    // for(int i = 63; i >= 0; --i)
    // {
    //     std::cout << ((word >> i) & 0x01);
    // }
    // std::cout << '\n';

    constexpr int precision = 10;
    std::cout << std::setw(width) << d2s(f) << std::setw(width) << d2fixed(f, precision)
              << std::setw(width) << d2exp(f, precision) << std::setw(width) << f;

    char buf[64];
    int idx = write_shortest_d(buf, f, '+', true, -1, 'e', true);
    buf[idx] = '\0';
    std::cout << std::setw(width) << buf;
    auto ret = std::to_chars(buf, buf + 64, f, std::chars_format::fixed);
    *ret.ptr = '\0';
    std::cout << std::setw(width) << buf;
    ret = std::to_chars(buf, buf + 64, f, std::chars_format::scientific);
    *ret.ptr = '\0';
    std::cout << std::setw(width) << buf;
    ret = std::to_chars(buf, buf + 64, f, std::chars_format::general);
    *ret.ptr = '\0';
    std::cout << std::setw(width) << buf << '\n';
  }
  return 0;
}
