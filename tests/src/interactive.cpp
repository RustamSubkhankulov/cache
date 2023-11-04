#include <iostream>
#include <cassert>
#include <cstddef>

#include "cache.hpp"
#include "cache_oper.hpp"

using namespace CACHES;
using std::size_t;

struct Run_options {
  size_t cache_size, elem_num;
  std::vector<int> key_seq;
};

namespace {

void on_input_format_error() {

  std::cout << "Invalid input format" << std::endl
            << "Input format:" << std::endl
            << "'cache_size N elem1 elem2 ... elemN', "
            << "where N is number of elements" << std::endl;
}

bool read_run_options(std::istream& is, Run_options* opt) {

  if (!(is >> opt->cache_size >> opt->elem_num)) {

    on_input_format_error();
    return false;
  }

  if (opt->cache_size < 0 || opt->elem_num < 0) {

    std::cout << "Error: cache size and number of elements "
              << "must be positive integers" << std::endl;
    return false;
  }

  opt->key_seq = read_key_seq<int>(opt->elem_num, is);
  if (is.fail()) {
    
    on_input_format_error();
    return false;
  }

  return true;
}

}; // anonymous namespace

int main() {

  Run_options opt;
  bool res = read_run_options(std::cin, &opt);
  if (!res) {
    return res;
  }

  Cache_LFU<int> lfu(opt.cache_size);
  Cache_PCA<int> pca(opt.cache_size, opt.key_seq.begin(), opt.key_seq.end());

  std::cout << "Hits statistics: \n";
  std::cout << "- LFU    :"  << test_cache(lfu, opt.key_seq) << std::endl;
  std::cout << "- Perfect: " << test_cache(pca, opt.key_seq) << std::endl;

  return 0;
}
