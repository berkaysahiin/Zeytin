/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014 - 2018 Axel Menzel <info@rttr.org>                           *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_DERIVED_INFO_FUNC_H_
#define RTTR_DERIVED_INFO_FUNC_H_

namespace rttr
{

namespace detail
{

 /*!
  * \brief A helper struct, which contains data about a class type.
  * This is used for the casting mechanism.
  */
struct derived_info
{
    void* m_ptr;    //!< A void pointer, which contains the address to an arbitrary instance.
    type m_type;    //!< The corresponding typ object to the \ref m_ptr
};

template<typename T>
static derived_info get_most_derived_info_impl(void* ptr)
{
    return (static_cast<T*>(ptr)->get_derived_info());
}

template<typename T>
static derived_info get_most_derived_info_impl_none(void* ptr)
{
    return {ptr, type::get<T>()};
}

using derived_func = derived_info(*)(void*);

/*!
 * \brief This function returns a function pointer to a function for retrieving the infos
 *        about the most derived type of an given instance.
 *
 */
template<typename T>
static derived_func get_most_derived_info_func()
{
    using raw_type = typename detail::raw_type<T>::type;

    if constexpr (has_get_derived_info_func<raw_type>::value)
        return get_most_derived_info_impl<raw_type>;
    else
        return get_most_derived_info_impl_none<raw_type>;
}


} // end namespace detail

} // end namespace rttr

#endif // RTTR_DERIVED_INFO_FUNC_H_
