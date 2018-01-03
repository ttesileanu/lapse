/** @file usetops.h
 *  @brief Some operations with unordered sets.
 */
#ifndef MISC_USETOPS_H_
#define MISC_USETOPS_H_

/// Take the difference of two @a unordered_set s.
template <class T>
boost::unordered_set<T> uset_difference(
    const boost::unordered_set<T>& s1,
    const boost::unordered_set<T>& s2)
{
  boost::unordered_set<T> result;
  typename boost::unordered_set<T>::const_iterator end1 = s1.end();
  typename boost::unordered_set<T>::const_iterator end2 = s2.end();

  for (typename boost::unordered_set<T>::const_iterator i = s1.begin();
       i != end1; ++i)
  {
    if (s2.find(*s1) == end2)
      result.insert(*s1);
  }

  return result;
}

#endif
