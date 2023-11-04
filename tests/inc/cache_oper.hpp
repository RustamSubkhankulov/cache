#pragma once 

#include <random>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "cache.hpp"

/*
 * Read 'elem_num' elements, representing keys of type 'Key' from input stream 'is'. 
 * Returns std::vector, containing read values.
 */
template <typename Key>
std::vector<Key> read_key_seq(std::size_t elem_num, std::istream& is) {

  std::vector<Key> key_seq;  

  while (is && key_seq.size() < elem_num) {
    Key temp;
    
    if (is >> temp)
      key_seq.push_back(std::move(temp));
  }

  return key_seq;
}

/*
 * Apply lookup_update to 'cache' for every key in 'key_seq' sequence of keys.
 * Returns number of hits.
 */
template <typename T, typename K, template<typename Type, typename Key> class Cache>
std::size_t test_cache(Cache<T, K>& cache, std::vector<K>& key_seq) {

  std::size_t hit_count = 0;
  for (auto& key : key_seq) {
    hit_count += (cache.lookup_update(key));
  }
  
  return hit_count;
}