#include <random>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>

#include "cache.hpp"
#include "cache_oper.hpp"
#include "auto_conf.hpp"

using namespace CACHES;

using std::size_t;
using std::ostream, std::cout, std::endl;

struct Test_options {
  size_t cache_size;
  size_t elem_num;
  int lower;
  int upper;
};

namespace {

std::vector<int> generate_key_seq(std::default_random_engine& rand_eng, 
                                               const Test_options& opt) {

  std::uniform_int_distribution<int> distr(opt.lower, opt.upper);

  std::vector<int> key_seq(opt.elem_num);  

  auto gen = [&distr, &rand_eng](){ return distr(rand_eng); };
  std::generate(key_seq.begin(), key_seq.end(), gen);

  return key_seq;
}

std::pair<size_t, size_t> run_generated_test_single(const Test_options& opt, 
                                       std::default_random_engine& rand_eng) {

  auto key_seq = generate_key_seq(rand_eng, opt);

  Cache_LFU<int> lfu(opt.cache_size);
  Cache_PCA<int> pca(opt.cache_size, key_seq.begin(), key_seq.end());

  return std::make_pair(test_cache(lfu, key_seq), test_cache(pca, key_seq));
}

std::ostream& operator<< (std::ostream& os, const Test_options& test_options) {

  os << "cache size = "      << test_options.cache_size << "; "
     << "elements number = " << test_options.elem_num   << "; ";

  os << "range of keys: [" << test_options.lower << "; " 
                           << test_options.upper << "]";

  return os;
}

const Test_options options[] = {

    {.cache_size =   4, .elem_num =   8, .lower = 0, .upper =   6},
    {.cache_size =   4, .elem_num =  16, .lower = 0, .upper =   9},

    {.cache_size =   8, .elem_num =  16, .lower = 0, .upper =  12},
    {.cache_size =   8, .elem_num =  32, .lower = 0, .upper =  20},

    {.cache_size =  16, .elem_num =  32, .lower = 0, .upper =  24},
    {.cache_size =  16, .elem_num =  64, .lower = 0, .upper =  36},

    {.cache_size =  32, .elem_num =  64, .lower = 0, .upper =  48},
    {.cache_size =  32, .elem_num = 128, .lower = 0, .upper =  72},

    {.cache_size =  64, .elem_num = 128, .lower = 0, .upper =  96},
    {.cache_size =  64, .elem_num = 256, .lower = 0, .upper = 144},

    {.cache_size = 128, .elem_num = 512, .lower = 0, .upper = 192},
    {.cache_size = 128, .elem_num = 512, .lower = 0, .upper = 288},
    
    {.cache_size = 1024, .elem_num = 25000, .lower = 0, .upper = 10000},
    {.cache_size = 1024, .elem_num = 50000, .lower = 0, .upper = 15000},
  };

};

int main() {

  std::random_device rand_devc;
  std::default_random_engine rand_eng(rand_devc());

  for (unsigned test_ind = 0; 
                test_ind < sizeof(options) / sizeof(Test_options); 
                test_ind++) {

    const Test_options& cur_test = options[test_ind];
    cout << "Test #" << test_ind + 1 << " " << cur_test << endl;

    auto results = run_generated_test_single(options[test_ind], rand_eng);
    cout << "- LFU    : " << results.first  << endl;
    cout << "- Perfect: " << results.second << endl;

    cout << endl;
  }

#ifdef GEN_EXTRA

  const Test_options additional_generated = {.cache_size = 2048, 
                                             .elem_num = 100000, 
                                             .lower = 0, 
                                             .upper = 50000};

  cout << "Additional tests: \n"; 
  cout << additional_generated << endl;

  for (unsigned test_ind = 0; 
                test_ind < GEN_EXTRA_NUM; 
                test_ind++) {

    std::cout << "Additional test #" << test_ind + 1 << std::endl; 

    auto results = run_generated_test_single(additional_generated);
    cout << "- LFU    : " << results.first  << endl;
    cout << "- Perfect: " << results.second << endl;

    cout << endl;
  }

#endif 

  return 0;
}
