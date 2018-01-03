/** @file callback.h
 *  @brief Defines a base class for a simple callback function.
 */
#ifndef MISC_CALLBACK_H_
#define MISC_CALLBACK_H_

struct Callback {
  /// Virtual destructor, for inheritance.
  virtual ~Callback() {}

  /** @brief This gets called to notify of progress.
   *
   *  @param progress Fraction of the procedure that was completed.
   *  @return @a false to stop the procedure, @a true otherwise.
   *
   *  Note that this may be called very frequently (i.e., for each line in
   *  a file that is read), so it's best to keep this function fast.
   */
  virtual bool operator()(float progress) = 0;
};

#endif
