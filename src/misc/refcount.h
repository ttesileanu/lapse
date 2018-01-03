/** @file refcount.h
 *  @brief Provides a framework for adding reference counting to arbitrary
 *         handle types.
 */
#ifndef MISC_REFCOUNT_H_
#define MISC_REFCOUNT_H_

#include <boost/call_traits.hpp>

/** @brief Base class for reference counted versions of handles.
 *
 *  Inherit from this class to provide a reference counted version of a handle.
 *  The @a Deleter should be a function that deletes an object referred to by a
 *  handle of type @a T.
 */
template <class T, void (*Deleter)(T)>
class RefCount {
 public:
  /// The kind of pointer/handle that is reference counted.
  typedef T value_type;
  /// Type used for passing to functions.
  typedef typename boost::call_traits<T>::param_type param_type;

  /// Constructor.
  RefCount() : count_(new int(1)) {}
  /// Constructor with initialization.
  explicit RefCount(param_type h) : handle_(h), count_(new int(1)) {}
  /// Destructor.
  virtual ~RefCount()
    { --(*count_); if (*count_ == 0) { Deleter(handle_); delete count_; } }

  /// Copy constructor.
  RefCount(const RefCount& alpha)
    : handle_(alpha.handle_), count_(alpha.count_) { ++(*count_); }

  /// Swap.
  friend void swap(RefCount& alpha, RefCount& beta) {
    using std::swap;

    swap(alpha.handle_, beta.handle_);
    swap(alpha.count_, beta.count_);
  }

  /// Assignment operator.
  RefCount& operator=(RefCount alpha) { swap(*this, alpha); return *this; }

  /// Access the pointer/handle.
  param_type operator()() const { return handle_; }

  /// Find whether this is the only handle to the object.
  bool isUnique() const { return (*count_ == 1); }

 protected:
  T     handle_;

 private:
  int*  count_;
};

#endif
