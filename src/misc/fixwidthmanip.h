/** @file fixwidthmanip.h
 *  @brief Defines a manipulator that allows the formatting of text within a
 *         fixed width.
 */
#ifndef FIXWIDTHMANIP_H_
#define FIXWIDTHMANIP_H_

namespace detail {

/// The structure generated by the manipulator function.
struct formatting_struct {
  int width;
};

/** @brief An object pretending to be a stream, collecting the text sent to it,
 *         and outputting it at destruction time.
 */
struct formatting_stream {
  /// Create the stream.
  formatting_stream(std::ostream& str, int w) : out(str), width(w) {}

  /** The @fixwidth function returns by value, so we need a copy constructor,
   *  but streams don't allow copies, so we work around that.
   */
  formatting_stream(const formatting_stream& other) :
      out(other.out), width(other.width), s(other.s.str()) {}
  /// The destructor performs the actual output to the original stream.
  ~formatting_stream() {
    std::string res = s.str();
    size_t line_start = 0;
    size_t last_wspace = 0;
    bool skip_ws = false;
    for (size_t i = 0; i < res.length(); ++i) {
      if (res[i] == '\n' || res[i] == '\r') {
        // allow explicit new lines
        out << res.substr(line_start, i - line_start + 1);
        line_start = i + 1;
        last_wspace = i;
        skip_ws = true;
      } else if (skip_ws && isspace(res[i])) {
        // skip whitespace at the beginning of new lines
        line_start = i + 1;
        last_wspace = i;
      } else if (i - line_start + 1 > width) {
        // split at blanks, or anywhere, if there are no blanks close enough
        if (last_wspace > line_start) {
          out << res.substr(line_start, last_wspace - line_start);
          line_start = last_wspace + 1;
          skip_ws = true;
        } else {
          out << res.substr(line_start, i - line_start);
          line_start = i;
          skip_ws = false;
        }
        out << std::endl;
      } else {
        if (isspace(res[i]))
          last_wspace = i;
        skip_ws = false;
      }
    }
    // make sure to output whatever's left on the last line
    if (line_start < res.length())
      out << res.substr(line_start);
  }

  /// Support the usual C++ stream interface.
  template <class T>
  formatting_stream& operator<<(T obj) { s << obj; return *this; }

  /// The stream that @a fixwidth was fed to.
  std::ostream& out;
  /// The maximum width that we're going to allow.
  int width;
  /// A string stream storing all the data sent to the object by @a operator<<.
  std::ostringstream s;

 private:
  /// Need a work-around to handle manipulators such as std::endl.
  typedef std::ostream& (*manip_handler_)(std::ostream&);

 public:
  /// Need a work-around to handle manipulators such as std::endl.
  formatting_stream& operator<<(manip_handler_ t) { s << t; return *this; }
};

formatting_stream operator<<(std::ostream& out, const formatting_struct& s)
{
  return formatting_stream{out, s.width};
}

} // namespace detail

/** @brief Manipulator that forces the remaining output to fit within the given
 *         @a width.
 *
 *  Note that this is not guaranteed to work properly unless the return value
 *  is used as a temporary.
 */
detail::formatting_struct fixwidth(int width = 80)
{
  return detail::formatting_struct{width};
}

#endif
