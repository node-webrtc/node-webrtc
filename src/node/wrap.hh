/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include "src/node/ref_ptr.hh"
#include "src/utilities/bidi_map.hh"
#include <type_traits>

namespace node_webrtc {

/**
 * Class that provides strong ownership of types `T`, caching based on a key
 * `U`.
 *
 * Assumes that `T` has a static member named `wrap()` with the same argument
 * instantiation.
 */
template <typename T, typename U, typename... V> class OwnedWrapImpl {
private:
  using BaseT = std::remove_pointer_t<T>;
  using Output = RefPtr<BaseT>;

public:
  OwnedWrapImpl() = default;
  ~OwnedWrapImpl() = default;
  OwnedWrapImpl(OwnedWrapImpl const &) = delete;
  OwnedWrapImpl(OwnedWrapImpl &&) = delete;
  OwnedWrapImpl &operator=(OwnedWrapImpl const &) = delete;
  OwnedWrapImpl &operator=(OwnedWrapImpl &&) = delete;

  Output GetOrCreate(V... args, U key) {
    return _map.computeIfAbsent(key, [key, args...]() {
      auto out = BaseT::wrap()->GetOrCreate(args..., key);
      return Output(out);
    });
  }

  Output Get(U key) { return _map.get(key).FromMaybe(nullptr); }

  void Release(T value) { _map.reverseRemove(value); }

  void clear() { _map.clear(); }

private:
  BidiMap<U, Output> _map;
};

/**
 * Class that provides weak ownership of types `T`, caching based on a key `U`.
 *
 * Used to provide a global cache of ObjectWrap objects, so that we don't have
 * to continuously re-construct them from their constituent parts every time.
 */
template <typename T, typename U, typename... V> class Wrap {
public:
  using owned = OwnedWrapImpl<T, U, V...>;
  Wrap(Wrap const &) = delete;
  Wrap(Wrap &&) = delete;
  Wrap &operator=(Wrap const &) = delete;
  Wrap &operator=(Wrap &&) = delete;
  Wrap() = delete;
  explicit Wrap(T (*create)(V..., U)) : _create(create) {}
  ~Wrap() = default;

  T GetOrCreate(V... args, U key) {
    return _map.computeIfAbsent(key, [this, key, args...]() {
      auto out = _create(args..., key);
      return out;
    });
  }

  T Get(U key) { return _map.get(key).FromMaybe(nullptr); }

  void Release(T value) { _map.reverseRemove(value); }

private:
  T (*_create)(V..., U);
  BidiMap<U, T> _map;
};

template <typename T>
using OwnedWrap = typename std::remove_reference_t<decltype(*T::wrap())>::owned;

} // namespace node_webrtc
