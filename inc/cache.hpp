#pragma once

#include <set>
#include <list>
#include <deque>
#include <stack>
#include <functional>
#include <unordered_map>

namespace CACHES {

/*
 * LFU cache template class. 
 */
template <typename T, typename Key = int>
class Cache_LFU {
public:

  using value_type = T;
  using key_type   = Key;
  using size_type = std::size_t;

  /*
   * Type of callable that is used to generate value stored in cache based on key.
   */
  using get_value = std::function<value_type(const key_type&)>;

private:

  struct data_elem_t {
    key_type key;
    value_type value;
    data_elem_t(key_type k, value_type v): key(k), value(v) {}
  };

  using list    = std::list<data_elem_t>;
  using data_it = typename list::iterator;

  struct set_elem_t {
    data_it iter;
    size_type counter;
    set_elem_t(data_it it, size_type ct): iter(it), counter(ct) {}
  };

  struct set_elem_cmp {

    bool operator() (const set_elem_t& lhs, const set_elem_t& rhs) const { 
      return lhs.counter < rhs.counter;
    }
  };
  
  size_type size_;
  list data_;

  /*
   * Hash table used for fast access to element in cache based on key.
   */
  using data_map = std::unordered_map<key_type, set_elem_t>;
  data_map hash_table_;

  /*
   * Set used for fast picking the element to be erased from cache -
   * 'least frequently used' element with smallest counter value - 
   * if there is not enough space for inserting new elements.
   */
  using elem_search_set = std::set<set_elem_t, set_elem_cmp>;  
  elem_search_set elem_search_set_;

public:

  explicit Cache_LFU(size_type size = 0)
  : size_(size) {}

  bool empty() const { return data_.empty(); }
  bool full() const { return (data_.size() == size_); }

  size_type size() const { return data_.size(); }
  size_type max_size() const { return size_; }

  void clear() {
   data_.clear();
   hash_table_.clear();
  }

  bool lookup_update(const key_type& key, 
                           get_value F = []([[maybe_unused]] const key_type& key){return value_type{};});

private:

  void insert_element(const key_type& key, get_value F) {

    data_.emplace_front(key, F(key));
    hash_table_.try_emplace(key, data_.begin(), 0);
    elem_search_set_.emplace(data_.begin(), 0);
  }
  
  void erase_element(typename elem_search_set::const_iterator elem) {

    data_it elim_data_it = elem->iter;

    elem_search_set_.erase(elem);
    hash_table_.erase(elim_data_it->key);
    data_.erase(elim_data_it);
  }

  /*
   * Update value of the elements' counter. Called on hit.
   */
  void update_element_counter(set_elem_t elem) {

    elem_search_set_.erase(elem);
    elem_search_set_.emplace(elem.iter, elem.counter + 1);
  }
};

template <typename T, typename Key>
bool Cache_LFU<T, Key>::lookup_update(const key_type& key, get_value F) {
  
  auto hit = hash_table_.find(key);
  if (hit == hash_table_.end()) { // not found

    if (size_ == 0)
      return false;

    if (full()) {
      erase_element(elem_search_set_.begin());
    }

    insert_element(key, F);
    return false;
  
  } else {

    update_element_counter(hit->second);
    return true;
  } 
};

/*
 * PCA - Perfect Caching Algorithm
 */
template <typename T, typename Key = int>
class Cache_PCA {
public:

  using value_type = T;
  using key_type   = Key;
  using size_type = std::size_t;

  /*
   * Type of callable that is used to generate value stored in cache based on key.
   */
  using get_value = std::function<value_type(const key_type&)>;

private:

  size_type size_;

  struct data_elem_t {
    key_type key;
    value_type value;
    data_elem_t(key_type k, value_type v): key(k), value(v) {}
  };

  using list = std::list<data_elem_t>;
  using data_it = typename list::iterator;
  list data_;

  /*
   * 'Future index'
   * Pair representing state of the element according to its future in key sequence.
   * first - wether element will occure in future.
   * second - its index in key sequence if first == true, otherwise 0.
   */ 
  using fidx = std::pair<bool, size_type>;

  static bool fidx_is_present(const fidx& arg) { return arg.first; }
  static size_type fidx_index(const fidx& arg) { return arg.second; }

  struct set_elem_t {
    data_it iter;
    size_type index;
    set_elem_t(data_it it, size_type idx): iter(it), index(idx) {}
  };

  struct set_elem_cmp {

    bool operator() (const set_elem_t& lhs, const set_elem_t& rhs) const { 
        return (lhs.index > rhs.index);
    }
  };

  /*
   * Set used for fast picking the farthest in future element in cache.
   */
  using data_fidx_set = std::set<set_elem_t, set_elem_cmp>;
  data_fidx_set data_fidx_;
  
  /*
   * Hash table for fast access to deque of 'future indexes' using key.
   */
  using key_seq_fidx_map = std::unordered_map<key_type, std::deque<size_type>>;
  key_seq_fidx_map key_seq_fidx_; 

  /*
   * Hash table used for fast access to element in cache based on key.
   */
  using data_map = std::unordered_map<key_type, data_it>;
  data_map hash_table_;

  /* 
   * Stack of iterators to elements in cache that will not occure in key sequence.
   * Such elements are first candidates to be erased if cache is appeared to be full on 
   * lookup_update().
   */
  std::stack<data_it> redundant_;

public:

  template <typename InputIt>
  Cache_PCA(size_type size, InputIt first, InputIt last) 
  : size_(size) {
      
    auto cur = first;
    size_type input_size = std::distance(first, last);

    for (size_type key_fidx = 0; key_fidx < input_size; key_fidx++) {
      key_seq_fidx_[*cur++].push_back(key_fidx);
    }
  }

  Cache_PCA(size_type size, const std::deque<key_type>& key_seq)
  : Cache_PCA(size, key_seq.begin(), key_seq.end()) { }

  bool empty() const { return data_.empty(); }
  bool full() const { return (data_.size() == size_); }

  size_type size() const { return data_.size(); }
  size_type max_size() const { return size_; }

  void clear() {
   data_.clear();
   hash_table_.clear();

   data_fidx_.clear();
   key_seq_fidx_.clear();
   redundant_.clear();
  }

  bool lookup_update(const key_type& key, 
                           get_value F = []([[maybe_unused]] const key_type& key){return value_type{};});

private:

  /*
   * Get element's next 'future index' next position in future sequence.
   */
  fidx get_elem_fidx(const key_type& key);
  
  /*
   * Picks element from cache and erases it and returns true. 
   * Or chooses not to insert currently looked up element since it will
   * not appear in future. In this case returns false.
   */
  bool free_space(fidx inserting_fidx);

  /* 
   * Pick element to be erased from cache using stack or redundant elements.
   */
  data_it get_elim_from_redundant() {

    data_it elem = redundant_.top();
    redundant_.pop();
    return elem;
  }

  /*
   * Pick element to be erased from cache if there are no redundant elements.
   */
  data_it get_elim_from_data() {

    auto farthest = farthest_elem();

    data_it elem = farthest->iter;
    data_fidx_.erase(farthest);
    return elem;
  }

  /*
   * Get element that is currently in cache that will appear the last in future.
   */
  typename data_fidx_set::iterator farthest_elem() const {
    return data_fidx_.begin();
  }

  void erase_element(data_it elem) {

    hash_table_.erase(elem->key);
    data_.erase(elem);
  }

  void insert_element(const key_type& key, get_value F) {

    data_.emplace_front(key, F(key));
    hash_table_.emplace(key, data_.begin());
  }

  /*
   * Performs necessary operations with supporting data structures 
   * on element insert.
   */
  void insert_update_internals(fidx inserting_fidx) {
    update_internals(inserting_fidx, data_.begin());
  }

  /*
   * Performs necessary operations with supporting data structures 
   * on element insert.
   */
  void hit_update_internals(const key_type& key, data_it elem) {

    data_fidx_.erase(set_elem_t{elem, key_seq_fidx_[key].front()});
    fidx next = get_elem_fidx(key);
    
    update_internals(next, elem);
  }

  /*
   * Add 'elem' to stack of redundants or set for fast picking.
   */ 
  void update_internals(fidx idx, data_it elem) {

    if (fidx_is_present(idx)) {
      data_fidx_.emplace(elem, fidx_index(idx));

    } else {
      redundant_.emplace(elem);
    }
  }
};

template <typename T, typename Key>
bool Cache_PCA<T, Key>::free_space(fidx inserting_fidx) {

  if (!fidx_is_present(inserting_fidx)) {
    return false;
  } 

  data_it elim;

  if (redundant_.size() != 0) {
    elim = get_elim_from_redundant();

  } else {

    if (inserting_fidx.second >= farthest_elem()->index) {
      return false;
    } 

    elim = get_elim_from_data();
  }

  erase_element(elim);
  return true;
}

template <typename T, typename Key>
typename Cache_PCA<T, Key>::fidx
Cache_PCA<T, Key>::get_elem_fidx(const key_type& key) {

  auto& deque = key_seq_fidx_[key];
  deque.pop_front();

  if (!deque.size()) {
    return std::make_pair(false, 0);
  }

  return std::make_pair(true, deque.front());
}

template <typename T, typename Key>
bool Cache_PCA<T, Key>::lookup_update(const key_type& key, get_value F) {
  
  auto hit = hash_table_.find(key);
  if (hit == hash_table_.end()) { // not found

    if (size_ == 0) {
      return false;
    }

    fidx inserting_fidx = get_elem_fidx(key);

    if (full() && !free_space(inserting_fidx)) {
        return false;
    }

    insert_element(key, F);

    insert_update_internals(inserting_fidx);
    return false;

  }

  hit_update_internals(key, hit->second);
  return true;
};

}; // namespace CACHES
