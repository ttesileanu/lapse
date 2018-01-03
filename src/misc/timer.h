/** @file timer.h
 *  @brief Defines an easy-to-use timer class and a frequency counter.
 */
#ifndef MISC_TIMER_H_
#define MISC_TIMER_H_

#include <boost/date_time/posix_time/posix_time.hpp>

/// A class for measuring durations of processes.
class Timer {
 public:
  /// Constructor.
  Timer() { reset(); }

  /// Reset the timer.
  void reset() {
    start_ = boost::posix_time::microsec_clock::universal_time();
  }
  /// Get elapsed time, as @a boost::posix_time::time_duration.
  boost::posix_time::time_duration getElapsedBoost() const {
    return boost::posix_time::microsec_clock::universal_time() - start_;
  }
  /// Get elapsed time in microseconds.
  long getElapsedUsec() const {
    return getElapsedBoost().total_microseconds();
  }
  /// Get elapsed time in seconds.
  double getElapsed() const {
    return static_cast<double>(getElapsedUsec()) / 1000000.0;
  }
  
 private:
  boost::posix_time::ptime      start_;
};

// XXX this class should be rewritten...
/// A class for measuring frequencies of repeated events.
class FrequencyCounter {
 public:
  /// Storage for a series of time events.
  typedef std::vector<boost::posix_time::ptime> Frames;

  /** @brief Constructor with averaging done over @a d seconds.
   *
   *  @param d This is the time for which event times are stored in the
   *           @a frames vector, which is also the time over which the frequency
   *           is averaged.
   */
  explicit FrequencyCounter(double d = 2.0) : delayReset_(d*1000000),
    countReset_(0) { }

  /** @brief Constructor with averaging done over @a n repetitions.
   *
   *  @param n This gives the size of the @a frames vector -- the frequency is
   *           averaged over @a n repetitions.
   */
  explicit FrequencyCounter(size_t n) : delayReset_(0.),
    countReset_(n) { }
  /** @brief Constructor with averaging done either over @a d seconds or @a n
   *         repetitions, whichever takes longer.
   */
  FrequencyCounter(double d, size_t n) : delayReset_(d*1000000),
    countReset_(n) { }

  /// Get the averaging time for the frequency.
  double getDelayReset() const { return delayReset_ / 1000000.0;  }// us -> s
  /// Get the number of event over which the frequency is averaged.
  size_t getCountReset() const { return countReset_; }

  ///
  size_t getTotalCount() const { return totalCount_; }
  void resetTotalCount() { totalCount_ = 0; }

  void setDelayReset(double d) {
    if (d < 0) d = 0;
    delayReset_ = d * 1000000; // d was in seconds, we want in us!
    checkReset_();
  }
  void setCountReset(size_t n) { countReset_ = n; checkReset_(); }

  const Frames& getFrames() const { return frames_; }

  double getFrameRate() const {
    if (frames_.size() < 2)
      return 0;
    double dt = (frames_.back() - frames_.front()).total_microseconds() /
      1000000.0;
    if (dt == 0)
      return 0;
    return (frames_.size() - 1)/ dt;
  }
  double operator()() const { return getFrameRate(); }

  void addFrame() {
    ++totalCount_;
    frames_.push_back(boost::posix_time::microsec_clock::universal_time());
    checkReset_();
  }
  void operator++() { addFrame(); }
  void operator++(int) { addFrame(); }

 private:
  void checkReset_() {
    if (frames_.empty())
      return;
    if (countReset_ > 0) {
      if (frames_.size() > countReset_) {
        // this is inefficient, but shouldn't be a problem
        frames_.erase(frames_.begin(), frames_.end() - countReset_);
      }
    }
    if (delayReset_ > 0) {
      // this is inefficient, could be replaced by a binary search...
      if ((frames_.back() - frames_.front()).total_microseconds()
            >= delayReset_) {
        Frames::iterator i = frames_.begin();
        while (i != frames_.end()) {
          if ((frames_.back() - (*i)).total_microseconds() < delayReset_)
            break;
          ++i;
        }
        frames_.erase(frames_.begin(), i);
      }
    }
  }

  Frames                                    frames_;
  double                                    delayReset_;
  size_t                                    countReset_;
  size_t                                    totalCount_;
};

#endif
