#pragma once

namespace error {
  struct not_in_alphabet : std::runtime_error {
    explicit not_in_alphabet(const char ch) : std::runtime_error("character not in alphabet: " + ch) {}
    explicit not_in_alphabet(const wchar_t ch) : std::runtime_error("character not in alphabet: " + ch) {}
    explicit not_in_alphabet(const int index) : std::runtime_error("index not in alphabet: " + index) {}
  };

  struct invalid_alphabet_sequence : std::runtime_error {
    explicit invalid_alphabet_sequence(const std::string type) : std::runtime_error("alphabet contains more elements than '" + type + "' can support") {}
  };

  struct null_iterator : std::runtime_error {
    explicit null_iterator(const std::string &message) : std::runtime_error("operation performed on null trie iterator: " + message) {}
  };
}

/* until c++17 */
namespace _std {

  template <class T>
  inline auto size(const T &seq) -> decltype(seq.size()) {
    return seq.size();
  }

  template <class T, size_t N>
  inline size_t size(const T(&arr)[N]) {
    return N;
  }

  inline size_t size(const char *arr) {
    return strlen(arr);
  }

  inline size_t size(const wchar_t *arr) {
    return wcslen(arr);
  }

}

template <class KeyT, class PredT = std::less<KeyT>>
class alphabet {
public:
  typedef KeyT key_type;
  typedef PredT compare_type;
  typedef std::vector<key_type> sequence_type;
  typedef typename sequence_type::const_iterator const_iterator;

private:
  compare_type _compare;
  sequence_type _alpha;
  const_iterator _begin, _end;

public:

  template <class SequenceT>
  explicit alphabet(const SequenceT &alpha) {
    std::set<key_type, compare_type> unique;
    for (auto &it : alpha)
      unique.insert(it);
    for (auto &it : unique)
      _alpha.push_back(it);

    if (_alpha.size() > std::pow(2, sizeof(key_type) * 8))
      throw error::invalid_alphabet_sequence(typeid(key_type).name());

    std::sort(_alpha.begin(), _alpha.end(), _compare);
    _begin = _alpha.begin();
    _end = _alpha.end();
  }

  int index_of(const key_type &ch) {
    return binary_search(0, _alpha.size(), ch);
  }

  key_type value_of(const int index) {
    if (index < 0 || index > _alpha.size())
      throw error::not_in_alphabet(index);
    return _alpha[index];
  }

  size_t size() const {
    return _alpha.size();
  }

private:

  int binary_search(size_t min, size_t max, const key_type &ch) {
    if (min >= max)
      return -1;
    size_t mid = (min + max) / 2;
    if (_compare(_alpha[mid], ch))
      return binary_search(mid + 1, max, ch);
    if (_compare(ch, _alpha[mid]))
      return binary_search(min, mid, ch);
    return int(mid);
  }

};

template <class KeyT, class ElemT, class PredT = std::less<KeyT>>
class trie;

template <class KeyT, class ElemT, class PredT = std::less<KeyT>>
class trie_node {
public:
  typedef KeyT key_type;
  typedef ElemT mapped_type;
  typedef PredT pred_type;
  typedef alphabet<key_type, pred_type> alphabet_type;
  typedef std::pair<bool, mapped_type> value_type;
  typedef trie_node<key_type, mapped_type, pred_type> self;

  friend trie<key_type, mapped_type, pred_type>;

private:
  value_type _value;
  key_type _key;
  self **_nodes;
  self *_parent;
  alphabet_type *_alphabet;

public:

  self *successor() {
    return successor(0);
  }

  self *predecessor() {
    return predecessor(int(_alphabet->size()) - 1);
  }

  template <class SequenceT>
  SequenceT key() {
    SequenceT key;
    self *next = this;
    while (next->_parent) {
      key.insert(key.begin(), next->_key);
      next = next->_parent;
    }
    return std::move(key);
  }

  mapped_type &value() {
    return _value.second;
  }

protected:

  trie_node() : _nodes(nullptr), _parent(nullptr), _alphabet(nullptr) {}

  ~trie_node() {
    if (_nodes) {
      for (int i = 0; i < _alphabet->size(); ++i)
        delete _nodes[i];
      delete[] _nodes;
    }
  }

  template <class SequenceT>
  mapped_type &operator[](const SequenceT &key) {
    self *node = traverse_and_create(key);
    node->_value.first = true;
    return node->_value.second;
  }

  template <class SequenceT>
  self *find(const SequenceT &key) {
    return traverse(key);
  }

  template <class SequenceT>
  bool has(const SequenceT &key) {
    self *node = traverse(key);
    return node ? node->_value.first : false;
  }

  template <class SequenceT>
  bool remove(const SequenceT &key) {
    self *node = traverse(key);
    if (!node)
      return false;

    node->_value = value_type();

    for (int i = _std::size(key) - 1; i >= 0; --i) {
      node = node->_parent;
      if (!node || !node->prune(index_of(key[i])))
        break;
    }
    return true;
  }

  void erase() {
    _value = value_type();
    self *next = this;
    while (next->_parent) {
      auto index = next->index_of(next->_key);
      next = next->_parent;
      if (!next->prune(index))
        break;
    }
  }

  void size(size_t &x) const {
    if (_value.first)
      ++x;
    if (_nodes)
      for (int i = 0; i < _alphabet->size(); ++i)
        if (_nodes[i])
          _nodes[i]->size(x);
  }

private:

  trie_node(const key_type &key, self *parent, alphabet_type *alphabet)
    : _key(key), _parent(parent), _alphabet(alphabet) {}

  self *predecessor(int start) {
    self *node;
    if (!((node = predecessor_in_children(start))))
      if (!((node = predecessor_in_parent())))
        node = root();
    return node;
  }

  self *predecessor_in_children(int start) {
    if (!_nodes)
      return nullptr;

    while (start >= 0 && _nodes[start] == nullptr)
      --start;

    if (start < 0)
      return nullptr;

    return _nodes[start]->active()
      ? _nodes[start]
      : _nodes[start]->predecessor();
  }

  self *predecessor_in_parent() {
    return _parent ? _parent->predecessor(index_of(_key) - 1) : nullptr;
  }

  self *successor(int start) {
    self *node;
    if (!((node = successor_in_children(start))))
      if (!((node = successor_in_parent())))
        node = root();
    return node;
  }

  self *successor_in_children(int start) {
    if (!_nodes)
      return nullptr;

    while (start < _alphabet->size() && _nodes[start] == nullptr)
      ++start;

    if (start >= _alphabet->size())
      return nullptr;

    return _nodes[start]->active()
      ? _nodes[start]
      : _nodes[start]->successor();
  }

  self *successor_in_parent() {
    return _parent ? _parent->successor(index_of(_key) + 1) : nullptr;
  }

  self *root() {
    return _parent ? _parent->root() : this;
  }

  int index_of(const key_type &key) {
    auto index = _alphabet->index_of(key);
    if (index < 0)
      throw error::not_in_alphabet(key);
    return index;
  }

  template <class SequenceT>
  self *traverse(const SequenceT &key) {
    self *node = this;
    for (size_t i = 0, size = _std::size(key); i < size; ++i)
      if (!((node = node->get_node(key[i]))))
        return nullptr;
    return node;
  }

  self *get_node(const key_type &key) {
    return _nodes ? _nodes[index_of(key)] : nullptr;
  }

  template <class SequenceT>
  self *traverse_and_create(const SequenceT &key) {
    self *next = this;
    for (size_t i = 0, size = _std::size(key); i < size; ++i)
      next = next->get_or_create_node(key[i]);
    return next;
  }

  self *get_or_create_node(const key_type &key) {
    init_nodes();
    self **node = &(_nodes[index_of(key)]);
    if (!*node)
      *node = new self(key, this, _alphabet);
    return *node;
  }

  void init_nodes() {
    if (!_nodes) {
      auto size = _alphabet->size();
      _nodes = new self*[size];
      for (int i = 0; i < size; ++i)
        _nodes[i] = nullptr;
    }
  }

  bool prune(const int key) {
    if (is_pruned(key))
      return true;
    if (!should_prune(_nodes[key]))
      return false;
    delete _nodes[key];
    _nodes[key] = nullptr;
    return true;
  }

  bool is_pruned(const int key) {
    return !_nodes || !_nodes[key];
  }

  bool should_prune(self *node) {
    if (!node)
      return true;
    if (node->_value.first)
      return false;
    if (node->_nodes)
      for (int i = 0; i < _alphabet->size(); ++i)
        if (node->_nodes[i])
          return false;
    return true;
  }

  bool active() {
    return _value.first;
  }

};

template <class TrieT>
class trie_iterator : public std::iterator<std::bidirectional_iterator_tag, TrieT> {
public:
  typedef trie_iterator iterator;
  typedef typename TrieT::pointer pointer;
  typedef typename TrieT::reference reference;

  explicit trie_iterator(pointer node) : _node(node) {}

  reference operator*() const {
    if (!_node)
      throw error::null_iterator("operator*()");
    return *_node;
  }

  pointer operator->() const {
    return _node;
  }

  iterator &operator++() {
    if (!_node)
      throw error::null_iterator("operator++()");
    _node = _node->successor();
    return *this;
  }

  iterator operator++(int) {
    iterator copy = *this;
    ++(*this);
    return copy;
  }

  iterator &operator--() {
    if (!_node)
      throw error::null_iterator("operator--()");
    _node = _node->predecessor();
    return *this;
  }

  iterator operator--(int) {
    iterator copy = *this;
    --(*this);
    return copy;
  }

  friend bool operator==(const iterator &left, const iterator &right) {
    return left._node == right._node;
  }

  friend bool operator!=(const iterator &left, const iterator &right) {
    return !(left == right);
  }

private:
  pointer _node;
};

template <class KeyT, class ElemT, class PredT = std::less<KeyT>>
class trie {
public:
  typedef KeyT key_type;
  typedef ElemT mapped_type;
  typedef PredT pred_type;
  typedef alphabet<key_type, pred_type> alphabet_type;
  typedef trie<key_type, mapped_type, pred_type> self;
  typedef trie_node<key_type, mapped_type, pred_type> value_type;

  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  typedef trie_iterator<self> iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  /*
  TODO:
  typedef const_trie_iterator<self> const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  */

  template <class SequenceT>
  explicit trie(const SequenceT &alpha) : _alphabet(alpha) {
    _root._alphabet = &_alphabet;
  }

  template <class SequenceT>
  mapped_type &operator[](const SequenceT &key) {
    return _root[key];
  }

  template <class SequenceT>
  iterator find(const SequenceT &key) {
    return iterator(_root.find(key));
  }

  template <class SequenceT>
  bool has(const SequenceT &key) {
    return _root.has(key);
  }

  template <class SequenceT>
  size_type erase(const SequenceT &key) {
    return _root.remove(key) ? 1 : 0;
  }

  iterator erase(iterator pos) {
    auto curr = pos++;
    curr->erase();
    return pos;
  }

  iterator erase(iterator first, iterator last) {
    auto it = first;
    while (it != last)
      it = erase(it);
    return it;
  }

  void clear() {
    _root = value_type();
  }

  size_t size() const {
    size_t x(0);
    _root.size(x);
    return x;
  }

  iterator begin() {
    value_type *node = _root.successor();
    return iterator(node ? node : &_root);
  }

  iterator end() {
    return iterator(&_root);
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }

  reverse_iterator rend() {
    return reverse_iterator(begin());
  }

protected:
  value_type _root;
  alphabet_type _alphabet;
};

