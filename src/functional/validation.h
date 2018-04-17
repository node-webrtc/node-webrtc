/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines Validation, modeled after
 * https://hackage.haskell.org/package/Validation
 */

#ifndef SRC_FUNCTIONAL_VALIDATION_H_
#define SRC_FUNCTIONAL_VALIDATION_H_

#include <cassert>
#include <functional>
#include <string>
#include <vector>

namespace node_webrtc {

typedef std::string Error;

typedef std::vector<Error> Errors;

/**
 * A Validation is similar to Either, in that it is a sum type whose "left"
 * values are a collection of errors and whose "right" values are some type T;
 * however, Validation can be used to represent values that need to be
 * validated. They can accrue multiple errors during the validation process.
 * @tparam T the type of value to validate
 */
template <typename T>
class Validation {
 public:
  // TODO(mroberts): This is no good.
  Validation(): _is_valid(false), _value(T()) {}

  /**
   * Construct a valid Validation. The value passed is assumed to be valid.
   * @param value the value to inject into the Validation
   */
  explicit Validation(const T& value): _is_valid(true), _value(value) {}

  /**
   * Validation forms an applicative. Apply a validation.
   * @tparam F the type of a function from T to S
   * @param f a Validation of a function from T to S
   * @return the result of applying the Validation
   */
  template <typename F>
  Validation<typename std::result_of<F(T)>::type> Apply(const Validation<F>& f) const {
    if (f.IsInvalid()) {
      auto errors = f.ToErrors();
      errors.insert(errors.end(), _errors.begin(), _errors.end());
      return Validation<typename std::result_of<F(T)>::type>::Invalid(errors);
    } else if (IsInvalid()) {
      return Validation<typename std::result_of<F(T)>::type>::Invalid(_errors);
    }
    return Validation<typename std::result_of<F(T)>::type>::Valid(f.UnsafeFromValid()(_value));
  }

  /**
   * Validation does not form a lawful Monad. Nevertheless, this is a useful
   * function to have.
   * @tparam S some type S
   * @param f a function from T to a Validation of S
   * @return a Validation of S
   */
  template <typename S>
  Validation<S> FlatMap(std::function<Validation<S>(T)> f) const {
    if (!_is_valid) {
      return Validation<S>::Invalid(_errors);
    }
    return f(_value);
  }

  /**
   * Eliminate a Validation. You must provide a default value to handle the
   * invalid case.
   * @param default_value the default value to use in the invalid case
   * @return the value in the Validation, if valid; otherwise, the default value
   */
  T FromValidation(const T& default_value) const {
    return _is_valid ? _value : default_value;
  }

  /**
   * Check whether or not the Validation is invalid.
   * @return true if the Validation is invalid; otherwise, false
   */
  bool IsInvalid() const {
    return !_is_valid;
  }

  /**
   * Check whether or not the Validation is valid.
   * @return true if the Validation is valid; otherwise, false
   */
  bool IsValid() const {
    return _is_valid;
  }

  /**
   * Validation forms a functor. Map a function over Validation.
   * @tparam F the type of a function from type T to S
   * @param f a function from type T to S
   * @return the mapped Validation
   */
  template <typename F>
  Validation<typename std::result_of<F(T)>::type> Map(F f) const {
    return _is_valid
        ? Validation<typename std::result_of<F(T)>::type>::Valid(f(_value))
        : Validation<typename std::result_of<F(T)>::type>::Invalid(_errors);
  }

  /**
   * Validation forms an alternative. If "this" is valid, return this; otherwise, that
   * @param that another Validation
   * @return this or that
   */
  Validation<T> Or(const Validation<T>& that) const {
    return _is_valid ? Validation<T>::Valid(_value) : that;
  }

  /**
   * Get the errors in a Validation.
   * @return errors
   */
  Errors ToErrors() const {
    return std::vector<Error>(_errors);
  }

  /**
   * Unsafely eliminate a Validation. This only works if the Validation is
   * valid.
   * @return the value in the Validation, if valid; otherwise, undefined
   */
  T UnsafeFromValid() const {
    assert(_is_valid);
    return _value;
  }

  /**
   * Construct an invalid Validation.
   * @param error error specifying why the Validation is invalid
   * @return an invalid Validation
   */
  static Validation<T> Invalid(const Error& error) {
    return Validation(false, std::vector<Error>({ error }));
  }

  /**
   * Construct an invalid Validation.
   * @param errors errors specifying why the Validation is invalid
   * @return an invalid Validation
   */
  static Validation<T> Invalid(const Errors& errors) {
    return Validation(false, errors);
  }

  /**
   * Validation does not form a lawful Monad. Nevertheless, this is a useful
   * function to have.
   * @param tt a Validation for a Validation of T
   */
  static Validation<T> Join(const Validation<Validation<T>>& tt) {
    return tt.template FlatMap<T>([](const Validation<T> t) { return t; });
  }

  /**
   * Sequence a vector of Validations into a Validation of a vector.
   * @param values a vector of Validations
   * @return a Validation of a vector
   */
  static Validation<std::vector<T>> Sequence(const std::vector<Validation<T>>& values) {
    auto errors = std::vector<Error>();
    auto valids = std::vector<T>();
    for (auto value : values) {
      if (value.IsValid()) {
        valids.push_back(value.UnsafeFromValid());
      } else {
        auto thoseErrors = value.ToErrors();
        errors.insert(errors.end(), thoseErrors.begin(), thoseErrors.end());
      }
    }
    return errors.empty() ? Validation<std::vector<T>>::Valid(valids) : Validation<std::vector<T>>::Invalid(errors);
  }

  /**
   * Construct a valid Validation.
   * @param value the value to inject into the Validation
   * @return a valid Validation
   */
  static Validation<T> Valid(const T& value) {
    return Validation(value);
  }

 private:
  Validation(const bool is_valid, const Errors& errors): _errors(errors), _is_valid(is_valid), _value(T()) {}

  Errors _errors;
  bool _is_valid;
  T _value;
};

}  // namespace node_webrtc

#endif  // SRC_FUNCTIONAL_VALIDATION_H_
