/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_BIDIMAP_H_
#define SRC_BIDIMAP_H_

#include <functional>
#include <map>
#include <utility>

#include "src/functional/maybe.h"
#include "src/functional/operators.h"

namespace node_webrtc {

/**
 * A BidiMap is a "bidirectional map" supporting get and set operations on both
 * keys and values.
 * @tparam K the type of keys
 * @tparam V the type of values
 */
template <typename K, typename V>
class BidiMap {
 public:
  /**
   * Construct an empty BidiMap.
   */
  BidiMap() = default;

  /**
   * Remove all keys and values from the BidiMap.
   */
  void clear() {
    _keyToValue.clear();
    _valueToKey.clear();
  }

  /**
   * Compute, set, and return a key's value if it's absent; otherwise, return
   * the key's value.
   * @param key
   * @param computeValue
   * @return the existing or newly set value
   */
  V computeIfAbsent(K key, std::function<V()> computeValue) {
    return get(key).Or([this, key, computeValue]() {
      auto value = computeValue();
      this->set(key, value);
      return value;
    });
  }

  /**
   * Get the key's value from the BidiMap.
   * @param key
   * @return Nothing if the key was not present
   */
  Maybe<V> get(K key) const {
    return has(key)
        ? Maybe<V>::Just(_keyToValue.at(key))
        : Maybe<V>::Nothing();
  }

  /**
   * Check if the BidiMap contains a value for the key.
   * @param key
   * @return true if the BidiMap contains a value for the key
   */
  bool has(K key) const {
    return _keyToValue.count(key) > 0;
  }

  /**
   * Remove the key and its value from the BidiMap.
   * @param key
   * @return Nothing if the key was not present
   */
  Maybe<V> remove(K key) {
    return [this, key](V value) {
      this->_keyToValue.erase(key);
      this->_valueToKey.erase(value);
      return value;
    } % get(key);
  }

  /**
   * Return a BidiMap with its keys and values swapped.
   * @return a BidiMap with its keys and values swapped
   */
  BidiMap<V, K> reverse() const {
    return BidiMap<V, K>(_valueToKey, _keyToValue);
  }

  /**
   * Compute, set, and return a value's key if it's absent; otherwise, return
   * the value's key.
   * @param value
   * @param computeKey
   * @return the existing or newly set key
   */
  K reverseComputeIfAbsent(V value, std::function<K()> computeKey) {
    return reverseGet(value).Or([this, value, computeKey]() {
      auto key = computeKey();
      this->reverseSet(value, key);
      return key;
    });
  }

  /**
   * Get the value's key from the BidiMap.
   * @param value
   * @return Nothing if the value was not present
   */
  Maybe<K> reverseGet(V value) const {
    return reverseHas(value)
        ? Maybe<K>::Just(_valueToKey.at(value))
        : Maybe<K>::Nothing();
  }

  /**
   * Check if the BidiMap contains a key for the value.
   * @param value
   * @return true if the BidiMap contains a key for the value
   */
  bool reverseHas(V value) const {
    return _valueToKey.count(value) > 0;
  }

  /**
   * Remove a value and its key from the BidiMap.
   * @param value
   * @return Nothing if the value was not present
   */
  Maybe<K> reverseRemove(V value) {
    return [this, value](K key) {
      this->_keyToValue.erase(key);
      this->_valueToKey.erase(value);
      return key;
    } % reverseGet(value);
  }

  /**
   * Set a value and its key in the BidiMap.
   * @param value
   * @param key
   * @return a pair of the previously set key (if any) and the previously set
   *         value (if any)
   */
  std::pair<Maybe<K>, Maybe<V>> reverseSet(V value, K key) {
    auto pair = std::make_pair(reverseGet(value), get(key));
    remove(key);
    _valueToKey[value] = key;
    _keyToValue[key] = value;
    return pair;
  }

  /**
   * Set a key and its value in the BidiMap.
   * @param key
   * @param value
   * @return a pair of the previously set value (if any) and the previously set
   *         key (if any)
   */
  std::pair<Maybe<V>, Maybe<K>> set(K key, V value) {
    auto pair = std::make_pair(get(key), reverseGet(value));
    reverseRemove(value);
    _keyToValue[key] = value;
    _valueToKey[value] = key;
    return pair;
  }

  /**
   * Construct a BidiMap from a map.
   * @param map
   * @return Nothing if the map contains duplicate values
   */
  static Maybe<BidiMap<K, V>> FromMap(std::map<K, V> map) {
    BidiMap<K, V> bidiMap;
    for (auto pair : map) {
      auto previousKey = bidiMap.reverseSet(pair.second, pair.first);
      if (previousKey.IsJust()) {
        return Maybe<BidiMap<K, V>>::Nothing();
      }
    }
    return Maybe<BidiMap<K, V>>::Just(bidiMap);
  }

 private:
  BidiMap(const std::map<K, V>& keyToValue, const std::map<V, K>& valueToKey)
    : _keyToValue(keyToValue), _valueToKey(valueToKey) {}

  std::map<K, V> _keyToValue;
  std::map<V, K> _valueToKey;
};

}  // namespace node_webrtc

#endif  // SRC_BIDIMAP_H_
