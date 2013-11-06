//
// Copyright 2012 (C). Alex Robenko. All rights reserved.
//

// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file embxx/container/StaticQueue.h
/// This file contains the definition and implementation of the static queue,
/// which also can be used as circular buffer.

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <iterator>

#include "embxx/util/Assert.h"

namespace embxx
{

namespace container
{

namespace static_queue_traits
{

/// @ingroup container
/// @brief Ignore error behaviour trait class of the BasicStaticQueue
/// @headerfile embxx/container/StaticQueue.h
class IgnoreError {};

/// @ingroup container
/// @brief Overwrite behaviour trait class of the BasicStaticQueue
/// @headerfile embxx/container/StaticQueue.h
class Overwrite {};


}  // namespace static_queue_traits


/// @addtogroup container
/// @{

/// @brief Base class for definition of static queues or circular buffers.
/// @details This class doesn't provide full functionality necessary for
///          proper operation of the queue - it doesn't provide any
///          functionality of inserting new elements. The main reason for
///          separating it from its derived class is to avoid machine code
///          duplication for functions that don't depend on the insertion
///          behavioural traits when template functions are instantiated.
///          Please refer to BasicStaticQueue class for full description
///          of the functionality.
/// @tparam T Type of the stored element.
/// @tparam TSize Size of the queue in number - maximum number of stored
///         elements.
/// @headerfile embxx/container/StaticQueue.h
template <
    typename T,
    std::size_t TSize>
class BasicStaticQueueBase
{
public:
    // Types

    /// @brief Type of the stored elements.
    typedef T ValueType;

    /// @brief Same as ValueType
    typedef ValueType value_type;

    /// @brief Size type.
    typedef std::size_t SizeType;

    /// @brief Same as SizeType
    typedef SizeType size_type;

    /// @brief Reference type to the stored elements.
    typedef ValueType& Reference;

    /// @brief Same as Reference
    typedef Reference reference;

    /// @brief Const reference type to the stored elements.
    typedef const ValueType& ConstReference;

    /// @brief Same as ConstReference;
    typedef ConstReference const_reference;

    /// @brief Pointer type to the stored elements.
    typedef ValueType* Pointer;

    /// @brief Same as Pointer
    typedef Pointer pointer;

    /// @brief Const pointer type to the stored elements.
    typedef const ValueType* ConstPointer;

    /// @brief Same as ConstPointer
    typedef ConstPointer const_pointer;

    /// @brief Internal array type to store the elements.
    typedef std::array<ValueType, TSize> ValueTypeArray;

    /// @brief Linearised iterator type
    typedef typename ValueTypeArray::iterator LinearisedIterator;

    /// @brief Const linearised iterator type
    typedef typename ValueTypeArray::const_iterator ConstLinearisedIterator;

    /// @brief Reverse linearised iterator type
    typedef typename ValueTypeArray::reverse_iterator ReverseLinearisedIterator;

    /// @brief Const reverse linearised iterator type
    typedef typename ValueTypeArray::const_reverse_iterator ConstReverseLinearisedIterator;

    /// @brief Linearised iterator range type - std::pair of (first, one-past-last) iterators.
    typedef std::pair<LinearisedIterator, LinearisedIterator> LinearisedIteratorRange;

    /// @brief Const version of IteratorRange
    typedef std::pair<ConstLinearisedIterator, ConstLinearisedIterator> ConstLinearisedIteratorRange;

    // Const iterator class
    class ConstIterator;

    /// @brief Same as ConstIterator
    typedef ConstIterator const_iterator;

    // Iterator class
    class Iterator;

    /// @brief Same as Iterator
    typedef Iterator iterator;

    // Member functions

    /// @brief Destructor
    /// @details The queue is cleared - the destructors of all the elements
    ///          in the queue are called
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       stored elements doesn't throw. Basic guarantee otherwise.
    ~BasicStaticQueueBase();

    /// @brief Copy assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide copy constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    BasicStaticQueueBase& operator=(const BasicStaticQueueBase& queue);

    /// @brief Move assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide move constructor.
    ///          If there is no move constructor defined, copy
    ///          constructor will be used. Please note that the provided
    ///          source queue itself is not updated, just the elements in
    ///          the queue may get invalid when move constructor is used
    ///          when elements are "moved".
    /// @param[in] queue Queue to copy elements from.
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the move constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    BasicStaticQueueBase& operator=(BasicStaticQueueBase&& queue);

    /// @brief Returns current size of the queue.
    /// @details When queue is empty 0 will be returned.
    ///          When queue is full the returned size is equal to the value
    ///          returned by capacity() member function.
    /// @return Unsigned value, size of the current queue.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    inline std::size_t size() const;

    /// @brief Returns capacity of the current queue
    /// @details Returns size of the queue provided in the class template
    ///          arguments
    /// @return Unsigned value, capacity of the queue.
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw.
    inline constexpr std::size_t capacity() const;

    /// @brief Returns whether the queue is empty.
    /// @return Returns true if and only if (size() == 0U) is true,
    ///         false otherwise.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    inline bool isEmpty() const;

    /// @brief Same as isEmpty()
    inline bool empty() const;

    /// @brief Returns whether the queue is full.
    /// @return Returns true if and only if (size() == capacity()) is true,
    ///         false otherwise.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    inline bool isFull() const;

    /// @brief Clears the queue from all the existing elements.
    /// @details The destructors of the stored elements will be called.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       stored elements doesn't throw. Basic guarantee otherwise.
    void clear();

    /// @brief Pop the element from the back of the queue.
    /// @details The destructor of the popped element is called. In case the
    ///          queue is empty, the request is ignored - no exception is
    ///          thrown.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popBack();

    /// @brief Same as popBack()
    inline void pop_back();

    /// @brief Pop number of the elements from the back of the queue.
    /// @details The destructors of the popped elements are called. In case the
    ///          queue is or gets empty in the process, no exception is
    ///          thrown.
    /// @param[in] count number of elements to pop.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popBack(std::size_t count);

    /// @brief Same as popBack(std::size_t)
    inline void pop_back(std::size_t count);

    /// @brief Pop the element from the front of the queue.
    /// @details The destructor of the popped element is called. In case the
    ///          queue is empty, the request is ignored - no exception is
    ///          thrown.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popFront();

    /// @brief Same as popFront()
    inline void pop_front();

    /// @brief Pop number of the elements from the front of the queue.
    /// @details The destructors of the popped elements are called. In case the
    ///          queue is or gets empty in the process, no exception is
    ///          thrown.
    /// @param[in] count number of elements to pop.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popFront(std::size_t count);

    /// @brief Same as popFront(std::size_t)
    inline void pop_front(std::size_t count);

    /// @brief Provides reference to the front element.
    /// @return Reference to the front element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference front();

    /// @brief Const version of front().
    ConstReference front() const;

    /// @brief Provides reference to the last element.
    /// @return Reference to the last element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference back();

    /// @brief Const version of back().
    ConstReference back() const;

    /// @brief Provides reference to the specified element
    /// @details In case the index is out of range, the returned value will be
    ///          a reference to some cell of the internal array. There is no
    ///          guarantee that the referenced object will be a valid one.
    /// @param[in] index Index of the object from the front of the queue.
    /// @return Reference to the stored element.
    /// @pre The index must be within the closed-open range of [0 - size()).
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference operator[](std::size_t index);

    /// @brief Const version of operator[]().
    ConstReference operator[](std::size_t index) const;

    /// @brief Provides reference to the specified element
    /// @details In case the index is out of range, the std::out_of_range
    ///          exception is thrown. This function is not suitable for the
    ///          pure embedded environment with small memory footprint
    ///          where exceptions mustn't be used. In this case use regular
    ///          boundary check combined with operator[].
    /// @param[in] index Index of the object from the front of the queue.
    /// @return Reference to the stored element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: Strong.
    Reference at(std::size_t index);

    /// @brief Const version of at().
    ConstReference at(std::size_t index) const;

    /// @brief Return index of the element in the current queue.
    /// @param[in] element Reference to the element.
    /// @return Non-negative value in case the element is stored in the
    ///         current queue, -1 otherwise.
    /// @post if returned index is not -1, than the operator[](<returned index>)
    ///       returns reference to the same parameter element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    int indexOf(ConstReference element) const;

    /// @brief Invalid iterator
    /// @details Returns value of invalid iterator, always equal to the end
    ///          of underlying array.
    /// @return Value of invalid iterator
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    LinearisedIterator invalidIter();

    /// @brief Const version of invalidIter()
    ConstLinearisedIterator invalidIter() const;

    /// @brief Invalid reverse iterator
    /// @details Returns value of invalid reverse iterator.
    /// @return Value of invalid reverse iterator
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    ReverseLinearisedIterator invalidReverseIter();

    /// @brief Const version of invalidReverseIter()
    ConstReverseLinearisedIterator invalidReverseIter() const;

    /// @brief Returns iterator to the linearised beginning.
    /// @details In case the queue is not linearised
    ///          the returned iterator will be the same as one returned
    ///          by the invalidIter() member function. Like in most standard
    ///          sequential containers the iterator may get invalid in
    ///          case the queue is updated in the middle of the iteration.
    /// @return Linearised iterator pointing to the first element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidIter()
    LinearisedIterator lbegin();

    /// @brief Same as clbegin().
    ConstLinearisedIterator lbegin() const;

    /// @brief Const version of lbegin().
    ConstLinearisedIterator clbegin() const;

    /// @brief Returns reverse iterator to the reverse linearised beginning.
    /// @details In case the queue is not linearised
    ///          the returned iterator will be the same as one returned
    ///          by the invalidReverseIter() member function. Like in most
    ///          standard sequential containers the iterator may get invalid
    ///          in case the queue is updated in the middle of the iteration.
    /// @return Reverse iterator pointing to the last element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidReverseIter()
    ReverseLinearisedIterator rlbegin();

    /// @brief Same as crlbegin().
    ConstReverseLinearisedIterator rlbegin() const;

    /// @brief Const version of rlbegin().
    ConstReverseLinearisedIterator crlbegin() const;

    /// @brief Returns iterator to the linearised end.
    /// @details In case the queue is not linearised the returned iterator
    ///          will the same as returned by invalidIter().
    /// @return Linearised iterator referring to the past-the-end element in
    ///         the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidIter()
    LinearisedIterator lend();

    /// @brief Same as clend().
    ConstLinearisedIterator lend() const;

    /// @brief Const version of lend().
    ConstLinearisedIterator clend() const;

    /// @brief Returns reverse iterator to the reverse linearised end.
    /// @details In case the queue is not linearised the returned iterator
    ///          will be the same as returned by invalidReverseIter().
    /// @return Reverse iterator pointing to the element right before the
    ///         first element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidReverseIter()
    ReverseLinearisedIterator rlend();

    /// @brief Same as crlend().
    ConstReverseLinearisedIterator rlend() const;

    /// @brief Const version of rlend();
    ConstReverseLinearisedIterator crlend() const;

    /// @brief Linearise internal elements.
    /// @details The elements of the queue are considered to be linearised
    ///          when the the pointer to every subsequent element will be
    ///          greater than the pointer to its predecessor. This queue is
    ///          implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. In this situation
    ///          it is not possible to safely iterate over the elements
    ///          without introducing a complexity of checking for this
    ///          particular case when the iterator is incremented or
    ///          decremented. The design decision was to create two types of
    ///          iterators: LinearisedIterator and Iterator. LinearisedIterator
    ///          may be safely used for iteration only when the queue is
    ///          linearised.
    ///          When linearising elements of the queue, the move constructor
    ///          of the type T will be called twice: first time to move the
    ///          element into the temporary queue and the second time to
    ///          move it back from the temporary queue into its new location
    ///          in the original queue.
    /// @warning There is a temporary queue of the same type (same size)
    ///          created on the stack. Avoid using this function if the queue
    ///          is too long and there is a  limit on the stack memory you
    ///          could use. Instead use arrayOne() and arrayTwo() functions to
    ///          get details of internal two parts and iterate over them.
    /// @post The queue elements are linearised.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case move constructor/desctructor
    ///       of the internal elements do not throw, Basic otherwise.
    /// @see isLinearised()
    /// @see arrayOne()
    /// @see arrayTwo()
    void linearise();

    /// @brief Returns whether the internal elements are linearised.
    /// @details The elements of the queue are considered to be linearised
    ///          when the the pointer to every subsequent element will be
    ///          greater than the pointer to its predecessor. In this case
    ///          it is possible to iterate over all the elements in a single
    ///          loop.
    /// @return true in case the element are linearised, false otherwise.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    bool isLinearised() const;

    /// @brief Get the first continuous array of the internal buffer.
    /// @details This queue is implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. As a result
    ///          it is not possible to properly iterate over all the elements
    ///          in a single loop. This function returns range of the linearised
    ///          iterators for the first continuous array of the internal buffer.
    /// @return Closed-open range (std::pair) of the iterators. Where the first
    ///         iterator refers to the front element of the queue while the
    ///         second iterator refers to either one-past-last or
    ///         one-past-some-middle element. In case the queue is empty,
    ///         both iterators are equal.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see linearise()
    /// @see isLinearised()
    /// @see arrayTwo()
    LinearisedIteratorRange arrayOne();

    /// @brief Const version of former arrayOne().
    ConstLinearisedIteratorRange arrayOne() const;

    /// @brief Get the second continuous array of the internal buffer.
    /// @details This queue is implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. As a result
    ///          it is not possible to properly iterate over all the elements
    ///          in a single loop. This function returns range of the iterators
    ///          for the second continuous array of the internal buffer.
    /// @return Closed-open range (std::pair) of the iterators. Where the first
    ///         iterator refers to the one of the middle elements of the queue
    ///         while the second iterator refers to either one-past-last
    ///         element. In case the queue is empty or linearised,
    ///         both iterators are equal to one returned by end() member
    ///         function.
    /// @post In case the queue is empty or linearised the first
    ///       iterator in the returned pair is equal to the second iterator
    ///       returned by the arrayOne() function.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see linearise()
    /// @see isLinearised()
    /// @see arrayOne()
    LinearisedIteratorRange arrayTwo();

    /// @brief Const version of former arrayTwo().
    ConstLinearisedIteratorRange arrayTwo() const;

    /// @brief Resize the queue.
    /// @details In case the new size is greater than the existing one,
    ///          new elements are added to the back of the queue (default
    ///          constructor is required). In case the new size is less
    ///          than existing one, existing elements are popped from the
    ///          back of the queue.
    /// @param[in] newSize New size of the queue.
    /// @pre New size can not be greater than the capacity of the queue.
    /// @post The queue is resized to new size or unchanged in case
    ///       precondition wasn't satisfied.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case constructor/desctructor
    ///       of the internal elements do not throw, Basic otherwise.
    void resize(std::size_t newSize);

    /// @brief Erase element.
    /// @details Erases element from specified position
    /// @param[in] pos Linearised iterator to the element to be erased
    /// @return Linearised iterator pointing to new location of
    ///         the next element after the erased one.
    /// @pre (pos != invalidIter())
    /// @pre pos is either in iterator range returned by arrayOne() or
    ///      returned by arrayTwo()
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy assignment operator
    ///       of the internal elements do not throw, Basic otherwise.
    LinearisedIterator erase(LinearisedIterator pos);

    /// @brief Returns iterator to the beginning.
    /// @details This iterator works on the non-linearised queue. It has extra
    ///          overhead to check for wrap arounds.
    /// @return Iterator pointing to the first element in the queue.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Iterator begin();

    /// @brief Same as cbegin();
    ConstIterator begin() const;

    /// @brief Const version of begin();
    ConstIterator cbegin() const;

    /// @brief Returns iterator to the end.
    /// @details This iterator works on the non-linearised queue. It has extra
    ///          overhead to check for wrap arounds.
    /// @return Iterator referring to the past-the-end element in
    ///         the queue.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Iterator end();

    /// @brief Same as cend();
    ConstIterator end() const;

    /// @brief Const version of end();
    ConstIterator cend() const;

protected:
    // Types

    /// Storage type for single element
    typedef
        typename std::aligned_storage<
            sizeof(ValueType),
            std::alignment_of<ValueType>::value
        >::type ValueStorageType;

    /// Internal place holder for the inserted elements
    typedef std::array<ValueStorageType, TSize> Array;

    /// @cond DOCUMENT_STATIC_ASSERT
    static_assert(sizeof(*(LinearisedIterator())) == sizeof(ValueType),
                                "Proper iterator increment must be supported");
    /// @endcond

    // Member functions

    /// @brief Default constructor.
    /// @details Initialises empty queue. The constructor is protected to
    ///          prevent creation of the queue without any insertion
    ///         functions which are implemented in the derived class.
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw.
    BasicStaticQueueBase();

    /// @brief Copy constructor.
    /// @details The constructor is protected to prevent
    ///          creation of the queue without any insertion functions
    ///          which are implemented in the derived class. The copy
    ///          constructor of the stored elements will be used to
    ///          copy all the elements from the provided queue.
    /// @note Thread safety: Depends on thread safety of copy constructor of
    ///       the stored elements.
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    BasicStaticQueueBase(const BasicStaticQueueBase& queue);

    /// @brief Move constructor.
    /// @details The constructor is protected to prevent
    ///          creation of the queue without any insertion functions
    ///          which are implemented in the derived class. The move
    ///          constructor of the stored elements will be used to
    ///          copy all the elements from the provided queue.
    /// @note Thread safety: Depends on thread safety of copy constructor of
    ///       the stored elements.
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    BasicStaticQueueBase(BasicStaticQueueBase&& queue);

    /// @brief Push elements to the back of the queue which is not full.
    /// @details This function is protected to be used by pushBack()
    ///          functions from the derived class.
    /// @param[in] value Value to insert, may be either l- or r-value.
    /// @pre The queue is not full.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy/move constructors
    ///       of the internal elements do not throw, Basic otherwise.
    template <typename U>
    void pushBackNotFull(U&& value);

    /// @brief Push elements to the front of the queue which is not full.
    /// @details This function is protected to be used by pushFront()
    ///          functions from the derived class.
    /// @param[in] value Value to insert, may be either l- or r-value.
    /// @pre The queue is not full.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy/move constructors
    ///       of the internal elements do not throw, Basic otherwise.
    template <typename U>
    void pushFrontNotFull(U&& value);

    template <typename U>
    LinearisedIterator insertNotFull(LinearisedIterator pos, U&& value);

private:

    // Base class for both Iterator and ConstIterator
    template <typename TDerived, typename TQueueType>
    class IteratorBase;

    template <typename TDerived, typename TQueueType>
    friend class BasicStaticQueueBase::IteratorBase;

    // Member functions

    /// @brief Internal function to create new value at specific index.
    /// @details It uses placement new together with copy or move
    ///          constructor of the element.
    /// @param[in] value value to be copied/moved.
    /// @param[in] index Index in the queue. May be greater than the size().
    /// @pre The index is within capacity of the queue.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy/move constructors
    ///       of the internal elements do not throw, Basic otherwise.
    template <typename U>
    void createValueAtIndex(U&& value, std::size_t index);

    /// @brief Auxiliary function to copy all the elements from source queue.
    /// @details It is used by both copy constructor and copy assignment
    ///          operator.
    /// @param[in] queue Queue to copy elements from.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    void assign(const BasicStaticQueueBase& queue);

    /// @brief R-value version of assign().
    void assign(BasicStaticQueueBase&& queue);

    // Member data
    Array array_; ///< Internal array to store elements.
    std::size_t startIdx_; ///< Index of the head of the queue.
    std::size_t count_; ///< Number of elements in the queue.
};

/// @brief Template class for definition of the static queues or circular
///        buffers.
/// @details The main distinction of this class from the alternatives
///          such as std::vector, std::deque or boost::circular_buffer is
///          that this implementation doesn't use dynamic memory allocation
///          and exceptions (except in "at()" member function). Hence, it
///          is suitable for use in pure embedded environment.
/// @tparam T Type of the stored element.
/// @tparam TSize Size of the queue in number - maximum number of stored
///         elements.
/// @tparam TTraits Traits that define an overflow behaviour when new
///         element is inserted into the queue. The TTraits must be a
///         class/struct that defines OverflowBehaviour as its public
///         internal type. The type can be either static_queue_traits::IgnoreError or
///         static_queue_traits::Overwrite. When overflow behaviour is "IgnoreError",
///         all the new push requests are ignored when the queue is full.
///         When overflow behaviour is "Overwrite", the first element at
///         the opposite side of the queue is overwritten with new element
///         when the queue is full. Please refer to definitions of
///         DefaultStaticQueueTraits and DefaultCircularBufferTraits for
///         the traits to choose between Queue and Circular buffer
///         functionalities.
/// @headerfile embxx/container/StaticQueue.h
template <
    typename T,
    std::size_t TSize,
    typename TTraits>
class BasicStaticQueue : public BasicStaticQueueBase<T, TSize>
{
    typedef BasicStaticQueueBase<T, TSize> Base;
public:
    // Types
    /// @brief Behaviour traits of the queue
    typedef TTraits Traits;

    /// @brief Overflow behaviour defined in the traits.
    typedef typename Traits::OverflowBehaviour OverflowBehaviour;

    /// @brief Type of the stored elements.
    typedef typename Base::ValueType ValueType;

    /// @brief Size type.
    typedef typename Base::SizeType SizeType;

    /// @brief Reference type to the stored elements.
    typedef typename Base::Reference Reference;

    /// @brief Const reference type to the stored elements.
    typedef typename Base::ConstReference ConstReference;

    /// @brief Pointer type to the stored elements.
    typedef typename Base::Pointer Pointer;

    /// @brief Const pointer type to the stored elements.
    typedef typename Base::ConstPointer ConstPointer;

    /// @brief Linearised iterator type
    typedef typename Base::LinearisedIterator LinearisedIterator;

    /// @brief Const linearised iterator type
    typedef typename Base::ConstLinearisedIterator ConstLinearisedIterator;

    /// @brief Reverse linearised iterator type
    typedef typename Base::ReverseLinearisedIterator ReverseLinearisedIterator;

    /// @brief Const reverse linearised iterator type
    typedef typename Base::ConstReverseLinearisedIterator ConstReverseLinearisedIterator;

    /// @brief Linearised iterator range type - std::pair of (first, one-past-last) iterators.
    typedef typename Base::LinearisedIteratorRange LinearisedIteratorRange;

    /// @brief Const version of IteratorRange
    typedef typename Base::ConstLinearisedIteratorRange ConstLinearisedIteratorRange;

    // Member functions
    /// @brief Default constructor.
    /// @details Creates empty queue.
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    BasicStaticQueue();

    /// @brief Copy constructor
    /// @details Copies all the elements from the provided queue using
    ///          copy constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the copy constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    BasicStaticQueue(const BasicStaticQueue& queue);

    /// @brief Move constructor
    /// @details Copies all the elements from the provided queue using
    ///          move constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the move constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case move constructor
    ///       of the internal elements do not throw, Basic otherwise.
    BasicStaticQueue(BasicStaticQueue&& queue);

    /// @brief Destructor
    /// @details The queue is cleared - the destructors of all the elements
    ///          in the queue are called
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       stored elements doesn't throw. Basic guarantee otherwise.
    ~BasicStaticQueue();

    /// @brief Copy assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide copy constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    BasicStaticQueue& operator=(const BasicStaticQueue& queue);

    /// @brief Move assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide move constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the move constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    BasicStaticQueue& operator=(BasicStaticQueue&& queue);

    /// @brief Add new element to the end of the queue.
    /// @details Uses copy constructor to copy the provided element.
    ///          The overflow behaviour depends on the traits of this queue
    ///          provided as template parameters. In case the queue is full and
    ///          "Overwrite" behaviour is requested in case of overflow, the
    ///          first element of the queue is popped, i.e. its destructor is
    ///          called.
    /// @param[in] value Value to insert
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    void pushBack(ConstReference value);

    /// @brief Same as pushBack(ConstReference)
    inline void push_back(ConstReference value);

    /// @brief R-value version of pushBack().
    void pushBack(ValueType&& value);

    /// @brief Same as pushBack(ValueType&&)
    inline void push_back(ValueType&& value);

    /// @brief Add new element to the front of the queue.
    /// @details Uses copy constructor to copy the provided element.
    ///          The overflow behaviour depends on the traits of this queue
    ///          provided as template parameters. In case the queue is full and
    ///          "Overwrite" behaviour is requested in case of overflow, the
    ///          last element of the queue is popped, i.e. its destructor is
    ///          called.
    /// @param[in] value Value to insert
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    void pushFront(ConstReference value);

    /// @brief Same as pushFront(ConstReference)
    inline void push_front(ConstReference value);

    /// @brief R-value version of pushFront().
    void pushFront(ValueType&& value);

    /// @brief Same as pushFront(ValueType&&)
    inline void push_front(ValueType&& value);

    /// @brief Insert new element at specified position.
    /// @details In case the queue is full, the insertion behaviour depends
    ///          on the overwrite behaviour trait of the queue. If the
    ///          trait is of type static_queue_traits::IgnoreError, the returned iterator
    ///          will be equal to the one returned by invalidIter(). In
    ///          case the trait is of type static_queue_traits::Overwrite, the last element
    ///          in the queue will be popped and new element is inserted
    ///          in the specified position.
    /// @param[in] pos Linearised iterator to the insert position.
    /// @param[in] value New value to insert.
    /// @return Linearised iterator to the newly inserted element.
    /// @pre (pos != invalidIter())
    /// @pre pos is either in iterator range returned by arrayOne() or
    ///      returned by arrayTwo() or equal to arrayTwo().second.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the move/copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    /// @see invalidIter()
    /// @see arrayOne()
    /// @see arrayTwo()
    LinearisedIterator insert(LinearisedIterator pos, ConstReference value);

    /// @brief R-value version of insert()
    LinearisedIterator insert(LinearisedIterator pos, ValueType&& value);

private:

    // Member functions

    /// Push back. Nothing is inserted if the queue is full.
    template <typename U>
    void pushBack(U&& value, const static_queue_traits::IgnoreError& behaviour);

    /// Push back. First element is overwritten if the queue is full.
    template <typename U>
    void pushBack(U&& value, const static_queue_traits::Overwrite& behaviour);

    /// Push front. Nothing is inserted if the queue is full.
    template <typename U>
    void pushFront(U&& value, const static_queue_traits::IgnoreError& behaviour);

    /// Push front. First element is overwritten if the queue is full.
    template <typename U>
    void pushFront(U&& value, const static_queue_traits::Overwrite& behaviour);

    /// Insert. Pop the last element if the queue is full.
    template <typename U>
    LinearisedIterator insert(
        LinearisedIterator pos,
        U&& value,
        const static_queue_traits::IgnoreError& behaviour);

    /// Insert. Pop the first element if the queue is full.
    template <typename U>
    LinearisedIterator insert(
        LinearisedIterator pos,
        U&& value,
        const static_queue_traits::Overwrite& behaviour);
};

/// @brief Base class for both Iterator and ConstIterator classes of
///        Static Queue.
/// @details This class was created to eliminate code duplication between
///          Iterator and ConstIterator classes.
/// @tparam TDerived Actual derived class (either Iterator or ConstIterator).
/// @tparam TQueueType Type of the queue (const or non-const).
template <typename T,
          std::size_t TSize>
template <typename TDerived, typename TQueueType>
class BasicStaticQueueBase<T, TSize>::IteratorBase
{
public:

    /// @brief Actual iterator class
    typedef TDerived Derived;

    /// @brief Type of the queue
    typedef TQueueType QueueType;

    /// @brief Type of the array iterator
    typedef decltype(QueueType().lbegin()) ArrayIterator;

    /// @brief Type of iterator category
    typedef typename std::iterator_traits<ArrayIterator>::iterator_category IteratorCategory;

    /// @brief Same as IteratorCategory
    typedef IteratorCategory iterator_category;

    /// @brief Type of the value referenced by the iterator
    typedef typename std::iterator_traits<ArrayIterator>::value_type ValueType;

    /// @brief Same as ValueType
    typedef ValueType value_type;

    /// @brief Type of the difference between two iterators
    typedef typename std::iterator_traits<ArrayIterator>::difference_type DifferenceType;

    /// @brief Same as DifferenceType
    typedef DifferenceType difference_type;

    /// @brief Type of the pointer to the value referenced by the iterator
    typedef typename std::iterator_traits<ArrayIterator>::pointer Pointer;

    /// @brief Same as Pointer
    typedef Pointer pointer;

    /// @brief Const pointer type
    typedef typename std::add_const<Pointer>::type ConstPointer;

    /// @brief Type of the reference to the value referenced by the iterator
    typedef typename std::iterator_traits<ArrayIterator>::reference Reference;

    /// @brief Same as Reference
    typedef Reference reference;

    /// @brief Const reference type
    typedef typename std::add_const<Reference>::type ConstReference;

    /// @brief Copy constructor is default.
    IteratorBase(const IteratorBase&) = default;

    /// @brief Copy assignment operator.
    /// @param other Other iterator
    /// @pre Other iterator must be iterator of the same queue.
    IteratorBase& operator=(const IteratorBase& other);

    /// @brief Pre increment operator.
    IteratorBase& operator++();

    /// @brief Post increment operator.
    IteratorBase operator++(int);

    /// @brief Pre decrement operator.
    IteratorBase& operator--();

    /// @brief Post-decrement operator.
    IteratorBase operator--(int);

    /// @brief Operator of adding constant value to the iterator
    /// @param value Value to be added
    IteratorBase& operator+=(DifferenceType value);

    /// @brief Operator of substructing constant value from the iterator
    /// @param value Value to be substructed
    IteratorBase& operator-=(DifferenceType value);

    /// @brief Operator +
    /// @details Creates new iterator object by adding constant to
    ///          current operator without changing it.
    /// @param value Value to be added
    IteratorBase operator+(DifferenceType value) const;

    /// @brief Operator -
    /// @details Creates new iterator object by substructing constant from
    ///          current operator without changing it.
    /// @param value Value to be substructed
    IteratorBase operator-(DifferenceType value) const;

    /// @brief Computes the distance between two iterators
    /// @param other Other iterator
    /// @pre other iterator must be of the same queue.
    DifferenceType operator-(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator==(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator!=(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<=(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>(const IteratorBase& other) const;

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>=(const IteratorBase& other) const;

    /// @brief Iterator dereference operator
    Reference operator*();

    /// @brief Const version of operator*
    ConstReference operator*() const;

    /// @brief Iterator dereference operator
    Pointer operator->();

    /// @brief Const version of operator->
    ConstPointer operator->() const;

    /// @brief Downcasting operator
    operator TDerived&();

    /// @brief Const downcasting operator
    operator const TDerived&() const;

protected:

    /// @brief Constructor
    /// @param queue Reference to queue
    /// @param iterator Low level array iterator
    IteratorBase(QueueType& queue, ArrayIterator iterator);

    QueueType& queue_; ///< Queue
    ArrayIterator iterator_; ///< Low level array iterator
};

/// @brief Proper const iterator class of the Static Queue
/// @details This iterator may be used to iterate over all the elements of
///          the Static Queue.
/// @extends BasicStaticQueueBase::IteratorBase
template <typename T,
          std::size_t TSize>
class BasicStaticQueueBase<T, TSize>::ConstIterator :
            public BasicStaticQueueBase<T, TSize>::
            template IteratorBase<Iterator, const BasicStaticQueueBase<T, TSize> >
{
    typedef typename BasicStaticQueueBase<T, TSize>::
        template IteratorBase<Iterator, const BasicStaticQueueBase<T, TSize> > Base;

public:

    /// @brief Queue type
    typedef typename Base::QueueType QueueType;

    /// @brief Low level array iterator type
    /// @details Equivalent to ConstLinearisedIterator
    typedef typename Base::ArrayIterator ArrayIterator;

    /// @brief Constructor
    /// @param queue Reference to the queue object
    /// @param iterator Low level iterator of type ConstLinearisedIterator
    ConstIterator(QueueType& queue, ArrayIterator iterator);

};

/// @brief Proper iterator class of the Static Queue
/// @details This iterator may be used to iterate over all the elements of
///          the Static Queue.
/// @extends BasicStaticQueueBase::IteratorBase
template <typename T,
          std::size_t TSize>
class BasicStaticQueueBase<T, TSize>::Iterator :
            public BasicStaticQueueBase<T, TSize>::
            template IteratorBase<Iterator, BasicStaticQueueBase<T, TSize> >
{
    typedef typename BasicStaticQueueBase<T, TSize>::
        template IteratorBase<Iterator, BasicStaticQueueBase<T, TSize> > Base;

public:

    /// @brief Queue type
    typedef typename Base::QueueType QueueType;

    /// @brief Low level array iterator type
    /// @details Equivalent to LinearisedIterator
    typedef typename Base::ArrayIterator ArrayIterator;

    /// @brief Constructor
    /// @param queue Reference to the queue object
    /// @param iterator Low level iterator of type LinearisedIterator
    Iterator(QueueType& queue, ArrayIterator iterator);

    /// @brief Conversion operator to ConstInterator type
    operator ConstIterator() const;
};


/// @brief behaviour traits to enforce double ended queue functionality
/// for BasicStaticQueue.
/// @details It is used as TTraits template parameter for BasicStaticQueue.
class DefaultStaticQueueTraits
{
public:
    typedef static_queue_traits::IgnoreError OverflowBehaviour;
};

/// @brief Partial template typedef for static double ended queue.
template <typename T, std::size_t TSize>
using StaticQueue = BasicStaticQueue<T, TSize, DefaultStaticQueueTraits>;

/// @brief behaviour traits to enforce circular buffer functionality
/// for BasicStaticQueue.
/// @details It is used as TTraits template parameter for BasicStaticQueue.
class DefaultCircularBufferTraits
{
public:
    typedef static_queue_traits::Overwrite OverflowBehaviour;
};

/// @brief Partial template typedef for static circular buffer.
template <typename T, std::size_t TSize>
using StaticCircularBuffer =
    BasicStaticQueue<T, TSize, DefaultCircularBufferTraits>;

/// @}

// Implementation part

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::~BasicStaticQueueBase()
{
    clear();
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>&
BasicStaticQueueBase<T, TSize>::operator=(const BasicStaticQueueBase& queue)
{
    if (this == &queue) {
        return *this;
    }

    assign(queue);
    return *this;
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>&
BasicStaticQueueBase<T, TSize>::operator=(BasicStaticQueueBase&& queue)
{
    if (this == &queue) {
        return *this;
    }

    assign(std::move(queue));
    return *this;
}

template <typename T, std::size_t TSize>
inline
std::size_t BasicStaticQueueBase<T, TSize>::size() const
{
    return count_;
}

template <typename T, std::size_t TSize>
inline
constexpr std::size_t BasicStaticQueueBase<T, TSize>::capacity() const
{
    return TSize;
}

template <typename T, std::size_t TSize>
inline
bool BasicStaticQueueBase<T, TSize>::isEmpty() const
{
    return (count_ == 0);
}

template <typename T, std::size_t TSize>
inline
bool BasicStaticQueueBase<T, TSize>::empty() const
{
    return isEmpty();
}

template <typename T, std::size_t TSize>
inline
bool BasicStaticQueueBase<T, TSize>::isFull() const
{
    return (count_ >= capacity());
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::clear()
{
    while (!isEmpty()) {
        popFront();
    }
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::popBack()
{
    if (isEmpty())
    {
        // Do nothing
        GASSERT(!"popBack() on empty queue");
        return;
    }

    Reference element = back();
    element.~T();

    --count_;
}

template <typename T, std::size_t TSize>
inline
void BasicStaticQueueBase<T, TSize>::pop_back()
{
    popBack();
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::popBack(std::size_t count)
{
    std::size_t countTmp = count;
    while ((!isEmpty()) && (countTmp > 0)) {
        popBack();
        --countTmp;
    }
}

template <typename T, std::size_t TSize>
inline
void BasicStaticQueueBase<T, TSize>::pop_back(std::size_t count)
{
    popBack(count);
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::popFront()
{
    if (isEmpty())
    {
        // Do nothing
        GASSERT(!"popFront() on empty queue");
        return;
    }

    Reference element = front();
    element.~T();

    --count_;
    ++startIdx_;
    if ((capacity() <= startIdx_) ||
        (isEmpty())) {
        startIdx_ = 0;
    }
}

template <typename T, std::size_t TSize>
inline
void BasicStaticQueueBase<T, TSize>::pop_front()
{
    popFront();
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::popFront(std::size_t count)
{
    std::size_t countTmp = count;
    while ((!isEmpty()) && (countTmp > 0)) {
        popFront();
        --countTmp;
    }
}

template <typename T, std::size_t TSize>
inline
void BasicStaticQueueBase<T, TSize>::pop_front(std::size_t count)
{
    popFront(count);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Reference
BasicStaticQueueBase<T, TSize>::front()
{
    return (*this)[0];
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReference
BasicStaticQueueBase<T, TSize>::front() const
{
    return (*this)[0];
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Reference
BasicStaticQueueBase<T, TSize>::back()
{
    auto constThis = static_cast<const BasicStaticQueueBase*>(this);
    return const_cast<Reference>(constThis->back());
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReference
BasicStaticQueueBase<T, TSize>::back() const
{
    if (!isEmpty()) {
        return (*this)[count_ - 1];
    }

    GASSERT(!"back() when queue is empty.");
    return (*this)[0];
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Reference
BasicStaticQueueBase<T, TSize>::operator[](std::size_t index)
{
    auto constThis = static_cast<const BasicStaticQueueBase*>(this);
    return const_cast<Reference>((*constThis)[index]);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReference
BasicStaticQueueBase<T, TSize>::operator[](std::size_t index) const
{
    std::size_t rawIdx = startIdx_ + index;
    while (capacity() <= rawIdx) {
        rawIdx = rawIdx - capacity();
    }

    auto cellAddr = &array_[rawIdx];
    return *(reinterpret_cast<ConstPointer>(cellAddr));
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Reference
BasicStaticQueueBase<T, TSize>::at(std::size_t index)
{
    auto constThis = static_cast<const BasicStaticQueueBase*>(this);
    return const_cast<Reference>(constThis->at(index));
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReference
BasicStaticQueueBase<T, TSize>::at(std::size_t index) const
{
    if (index >= size()) {
        throw std::out_of_range(std::string("Index is out of range"));
    }
    return (*this)[index];
}

template <typename T, std::size_t TSize>
int BasicStaticQueueBase<T, TSize>::indexOf(ConstReference element) const
{
    ConstPointer elementPtr = &element;
    typedef typename Array::const_pointer ArrayCellPtrType;
    auto cellPtr = reinterpret_cast<ArrayCellPtrType>(elementPtr);
    if ((cellPtr < &array_.front()) ||
        (&array_.back() < cellPtr)) {
        // invalid, element is not within array boundaries
        return -1;
    }

    auto rawIdx = elementPtr - reinterpret_cast<ConstPointer>(&array_.front());
    std::size_t actualIdx = capacity(); // invalid value
    if (rawIdx < startIdx_) {
        actualIdx = (capacity() - startIdx_) + rawIdx;
    }
    else {
        actualIdx = rawIdx - startIdx_;
    }

    if (size() <= actualIdx)
    {
        // Element is out of range
        return -1;
    }

    return actualIdx;
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIterator
BasicStaticQueueBase<T, TSize>::invalidIter()
{
    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->end();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIterator
BasicStaticQueueBase<T, TSize>::invalidIter() const
{
    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->end();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::invalidReverseIter()
{
    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->rend();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::invalidReverseIter() const
{
    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->rend();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIterator
BasicStaticQueueBase<T, TSize>::lbegin()
{
    if (!isLinearised()) {
        return invalidIter();
    }

    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->begin() + startIdx_;
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIterator
BasicStaticQueueBase<T, TSize>::lbegin() const
{
    return clbegin();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIterator
BasicStaticQueueBase<T, TSize>::clbegin() const
{
    if (!isLinearised()) {
        return invalidIter();
    }

    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->cbegin() + startIdx_;
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::rlbegin()
{
    if (!isLinearised()) {
        return invalidReverseIter();
    }

    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->rbegin() +
        (capacity() - (startIdx_ + size()));
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::rlbegin() const
{
    return crlbegin();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::crlbegin() const
{
    if (!isLinearised()) {
        return invalidReverseIter();
    }

    auto reinterpretArrayPtr =
        reinterpret_cast<const ValueTypeArray*>(&array_);
    return reinterpretArrayPtr->crbegin() +
        (capacity() - (startIdx_ + size()));
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIterator
BasicStaticQueueBase<T, TSize>::lend()
{
    if (!isLinearised()) {
        return invalidIter();
    }

    return lbegin() + size();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIterator
BasicStaticQueueBase<T, TSize>::lend() const
{
    return clend();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIterator
BasicStaticQueueBase<T, TSize>::clend() const
{
    if (!isLinearised()) {
        return invalidIter();
    }

    return clbegin() + size();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::rlend()
{
    if (!isLinearised()) {
        return invalidReverseIter();
    }

    return rlbegin() + size();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::rlend() const
{
    return crlend();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstReverseLinearisedIterator
BasicStaticQueueBase<T, TSize>::crlend() const
{
    if (!isLinearised()) {
        return invalidReverseIter();
    }

    return rlbegin() + size();
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::linearise()
{
    if (isLinearised()) {
        // Nothing to do
        return;
    }

    BasicStaticQueueBase tempQueue;
    while (!isEmpty()) {
        tempQueue.pushBackNotFull(std::move(front()));
        popFront();
    }

    (*this) = std::move(tempQueue);
}

template <typename T, std::size_t TSize>
bool BasicStaticQueueBase<T, TSize>::isLinearised() const
{
    return (isEmpty() || ((startIdx_ + size()) <= capacity()));
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIteratorRange
BasicStaticQueueBase<T, TSize>::arrayOne()
{
    auto reinterpretArrayPtr =
        reinterpret_cast<ValueTypeArray*>(&array_);

    auto constThis = static_cast<const BasicStaticQueueBase*>(this);
    auto constRange = constThis->arrayOne();
    auto beginIter = reinterpretArrayPtr->begin() +
        (constRange.first - reinterpretArrayPtr->cbegin());
    auto endIter = reinterpretArrayPtr->begin() +
        (constRange.second - reinterpretArrayPtr->cbegin());
    return LinearisedIteratorRange(beginIter, endIter);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIteratorRange
BasicStaticQueueBase<T, TSize>::arrayOne() const
{
    auto reinterpretArrayPtr =
        reinterpret_cast<const ValueTypeArray*>(&array_);

    auto beginIter = reinterpretArrayPtr->begin() + startIdx_;
    if (isEmpty()) {
        GASSERT(startIdx_ == 0);
        return ConstLinearisedIteratorRange(beginIter, beginIter);
    }

    auto endIter = reinterpretArrayPtr->end();
    std::size_t rawEndIdx = startIdx_ + size();
    if (rawEndIdx < capacity()) {
        endIter = beginIter + size();
    }
    return ConstLinearisedIteratorRange(beginIter, endIter);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIteratorRange
BasicStaticQueueBase<T, TSize>::arrayTwo()
{
    auto reinterpretArrayPtr =
        reinterpret_cast<ValueTypeArray*>(&array_);
    auto constThis = static_cast<const BasicStaticQueueBase*>(this);
    auto constRange = constThis->arrayTwo();
    auto beginIter = reinterpretArrayPtr->begin() +
        (constRange.first - reinterpretArrayPtr->cbegin());
    auto endIter = reinterpretArrayPtr->begin() +
        (constRange.second - reinterpretArrayPtr->cbegin());
    return LinearisedIteratorRange(beginIter, endIter);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstLinearisedIteratorRange
BasicStaticQueueBase<T, TSize>::arrayTwo() const
{
    auto reinterpretArrayPtr =
            reinterpret_cast<const ValueTypeArray*>(&array_);

    if (isLinearised()) {
        auto iter = arrayOne().second;
        return ConstLinearisedIteratorRange(iter, iter);
    }

    auto beginIter = reinterpretArrayPtr->begin();
    auto endIter = beginIter + ((startIdx_ + size()) - capacity());
    return ConstLinearisedIteratorRange(beginIter, endIter);
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::resize(std::size_t newSize)
{
    if (capacity() < newSize) {
        return;
    }

    if (size() <= newSize) {
        while (size() < newSize) {
            pushBackNotFull(ValueType());
        }

        return;
    }

    // New requested size is less than existing now
    popBack(size() - newSize);
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::LinearisedIterator
BasicStaticQueueBase<T, TSize>::erase(LinearisedIterator pos)
{
    GASSERT(pos != invalidIter());
    GASSERT(!isEmpty());
    auto rangeOne = arrayOne();
    auto rangeTwo = arrayTwo();

    auto isInRangeFunc = [](LinearisedIterator pos, const LinearisedIteratorRange range) -> bool
        {
            return ((range.first <= pos) && (pos < range.second));
        };

    GASSERT(isInRangeFunc(pos, rangeOne) ||
           isInRangeFunc(pos, rangeTwo));

    if (isInRangeFunc(pos, rangeOne)) {
        std::move_backward(rangeOne.first, pos, pos + 1);

        popFront();
        if (!isEmpty()) {
            return pos + 1;
        }

        return arrayOne().first;
    }

    if (isInRangeFunc(pos, rangeTwo)) {
        std::move(pos + 1, rangeTwo.second, pos);
        popBack();
        if (!isLinearised()) {
            return pos;
        }
        return arrayOne().second;
    }

    GASSERT(!"Invalid iterator is used");
    return invalidIter();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Iterator
BasicStaticQueueBase<T, TSize>::begin()
{
    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return Iterator(*this, reinterpretArrayPtr->begin());
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstIterator
BasicStaticQueueBase<T, TSize>::begin() const
{
    return cbegin();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstIterator
BasicStaticQueueBase<T, TSize>::cbegin() const
{
    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&array_);
    return ConstIterator(*this, reinterpretArrayPtr->cbegin());
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::Iterator
BasicStaticQueueBase<T, TSize>::end()
{
    auto reinterpretArrayPtr = reinterpret_cast<ValueTypeArray*>(&array_);
    return Iterator(*this, reinterpretArrayPtr->begin() + size());
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstIterator
BasicStaticQueueBase<T, TSize>::end() const
{
    return cend();
}

template <typename T, std::size_t TSize>
typename BasicStaticQueueBase<T, TSize>::ConstIterator
BasicStaticQueueBase<T, TSize>::cend() const
{
    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&array_);
    return ConstIterator(*this, reinterpretArrayPtr->cbegin() + size());
}

template <typename T, std::size_t TSize>
template <typename U>
void BasicStaticQueueBase<T, TSize>::createValueAtIndex(
    U&& value,
    std::size_t index)
{
    GASSERT(index < capacity());
    Reference elementRef = (*this)[index];
    auto elementPtr = new(&elementRef) ValueType(std::forward<U>(value));
    static_cast<void>(elementPtr);
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::BasicStaticQueueBase()
    : startIdx_(0),
      count_(0)
{
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::BasicStaticQueueBase(
    BasicStaticQueueBase&& queue)
    : startIdx_(0),
      count_(0)
{
    assign(std::move(queue));
}

template <typename T, std::size_t TSize>
template <typename U>
void BasicStaticQueueBase<T, TSize>::pushBackNotFull(U&& value)
{
    GASSERT(!isFull());
    createValueAtIndex(std::forward<U>(value), size());
    ++count_;
}

template <typename T, std::size_t TSize>
template <typename U>
void BasicStaticQueueBase<T, TSize>::pushFrontNotFull(U&& value)
{
    GASSERT(!isFull());
    createValueAtIndex(std::forward<U>(value), capacity() - 1);
    if (startIdx_ == 0) {
        startIdx_ = capacity() - 1;
    }
    else {
        --startIdx_;
    }

    ++count_;
}

template <typename T, std::size_t TSize>
template <typename U>
typename BasicStaticQueueBase<T, TSize>::LinearisedIterator
BasicStaticQueueBase<T, TSize>::insertNotFull(LinearisedIterator pos, U&& value)
{
    GASSERT(!isFull());
    GASSERT(pos != invalidIter());
    auto rangeOne = arrayOne();
    auto rangeTwo = arrayTwo();

    if (pos == rangeOne.first) {
        pushFrontNotFull(std::forward<U>(value));
        return arrayOne().first;
    }

    if (pos == rangeTwo.second) {
        pushBackNotFull(std::forward<U>(value));
        return arrayTwo().second - 1;
    }

    auto isInRangeFunc = [](LinearisedIterator pos, const LinearisedIteratorRange range) -> bool
        {
            return ((range.first <= pos) && (pos < range.second));
        };

    GASSERT(isInRangeFunc(pos, rangeOne) ||
            isInRangeFunc(pos, rangeTwo));

    if (isInRangeFunc(pos, rangeOne)) {
        auto firstElem = std::move(front());

        std::move(rangeOne.first + 1, pos, rangeOne.first);
        *pos = std::forward<U>(value);
        pushFrontNotFull(std::move(firstElem));
        return pos;
    }

    if (isInRangeFunc(pos, rangeTwo)) {
        auto lastElem = std::move(back());
        std::move_backward(pos, rangeTwo.second - 1, rangeTwo.second);
        *pos = std::forward<U>(value);
        pushBackNotFull(std::move(lastElem));
        return pos;
    }

    return invalidIter();
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::BasicStaticQueueBase(
    const BasicStaticQueueBase& queue)
    : startIdx_(0),
      count_(0)
{
    assign(queue);
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::assign(
    const BasicStaticQueueBase& queue)
{
    clear();
    auto pushBackFunc = [this](const ConstLinearisedIteratorRange& range)
        {
            for (auto iter = range.first; iter != range.second; ++iter) {
                this->pushBackNotFull(*iter);
            }
        };

    pushBackFunc(queue.arrayOne());
    pushBackFunc(queue.arrayTwo());
}

template <typename T, std::size_t TSize>
void BasicStaticQueueBase<T, TSize>::assign(
    BasicStaticQueueBase&& queue)
{
    clear();
    auto pushBackFunc = [this](const LinearisedIteratorRange& range)
        {
            for (auto iter = range.first; iter != range.second; ++iter) {
                this->pushBackNotFull(std::move(*iter));
            }
        };

    pushBackFunc(queue.arrayOne());
    pushBackFunc(queue.arrayTwo());
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>::BasicStaticQueue()
    : Base()
{
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>::BasicStaticQueue(
    const BasicStaticQueue& queue)
    : Base(queue)
{
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>::BasicStaticQueue(
    BasicStaticQueue&& queue)
    : Base(std::move(queue))
{
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>::~BasicStaticQueue()
{
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>&
BasicStaticQueue<T, TSize, TTraits>::operator=(
    const BasicStaticQueue& queue)
{
    Base::operator=(queue);
    return *this;
}

template <typename T, std::size_t TSize, typename TTraits>
BasicStaticQueue<T, TSize, TTraits>&
BasicStaticQueue<T, TSize, TTraits>::operator=(
    BasicStaticQueue&& queue)
{
    Base::operator=(std::move(queue));
    return *this;
}


template <typename T, std::size_t TSize, typename TTraits>
void BasicStaticQueue<T, TSize, TTraits>::pushBack(ConstReference value)
{
    pushBack(value, typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
inline
void BasicStaticQueue<T, TSize, TTraits>::push_back(ConstReference value)
{
    pushBack(value);
}

template <typename T, std::size_t TSize, typename TTraits>
void BasicStaticQueue<T, TSize, TTraits>::pushBack(ValueType&& value)
{
    pushBack(std::move(value), typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
inline
void BasicStaticQueue<T, TSize, TTraits>::push_back(ValueType&& value)
{
    pushBack(std::move(value));
}

template <typename T, std::size_t TSize, typename TTraits>
void BasicStaticQueue<T, TSize, TTraits>::pushFront(ConstReference value)
{
    pushFront(value, typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
inline
void BasicStaticQueue<T, TSize, TTraits>::push_front(ConstReference value)
{
    pushFront(value);
}

template <typename T, std::size_t TSize, typename TTraits>
void BasicStaticQueue<T, TSize, TTraits>::pushFront(ValueType&& value)
{
    pushFront(std::move(value), typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
inline
void BasicStaticQueue<T, TSize, TTraits>::push_front(ValueType&& value)
{
    pushFront(std::move(value));
}

template <typename T, std::size_t TSize, typename TTraits>
typename BasicStaticQueue<T, TSize, TTraits>::LinearisedIterator
BasicStaticQueue<T, TSize, TTraits>::insert(LinearisedIterator pos, ConstReference value)
{
    return insert(pos, value, typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
typename BasicStaticQueue<T, TSize, TTraits>::LinearisedIterator
BasicStaticQueue<T, TSize, TTraits>::insert(LinearisedIterator pos, ValueType&& value)
{
    return insert(pos, std::move(value), typename TTraits::OverflowBehaviour());
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
void BasicStaticQueue<T, TSize, TTraits>::pushBack(
    U&& value,
    const static_queue_traits::IgnoreError& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        return;
    }

    Base::pushBackNotFull(std::forward<U>(value));
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
void BasicStaticQueue<T, TSize, TTraits>::pushBack(
    U&& value,
    const static_queue_traits::Overwrite& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        Base::popFront();
    }

    Base::pushBackNotFull(std::forward<U>(value));
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
void BasicStaticQueue<T, TSize, TTraits>::pushFront(
    U&& value,
    const static_queue_traits::IgnoreError& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        return;
    }

    Base::pushFrontNotFull(std::forward<U>(value));
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
void BasicStaticQueue<T, TSize, TTraits>::pushFront(
    U&& value,
    const static_queue_traits::Overwrite& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        Base::popBack();
    }

    Base::pushFrontNotFull(std::forward<U>(value));
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
typename BasicStaticQueue<T, TSize, TTraits>::LinearisedIterator
BasicStaticQueue<T, TSize, TTraits>::insert(
    LinearisedIterator pos,
    U&& value,
    const static_queue_traits::IgnoreError& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        return Base::invalidIter();
    }

    return Base::insertNotFull(pos, std::forward<U>(value));
}

template <typename T, std::size_t TSize, typename TTraits>
template <typename U>
typename BasicStaticQueue<T, TSize, TTraits>::LinearisedIterator
BasicStaticQueue<T, TSize, TTraits>::insert(
    LinearisedIterator pos,
    U&& value,
    const static_queue_traits::Overwrite& behaviour)
{
    static_cast<void>(behaviour);

    if (Base::isFull()) {
        if (pos == Base::arrayTwo().second) {
            return Base::invalidIter();
        }
        Base::popBack();
    }

    return Base::insertNotFull(pos, std::forward<U>(value));
}


template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>&
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator=(
    const IteratorBase& other)
{
    GASSERT(&queue_ == &other.queue_);
    iterator_ = other.iterator_; // No need to check for self assignment
    return *this;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>&
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator++()
{
    ++iterator_;
    return *this;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator++(int)
{
    IteratorBase copy(*this);
    ++iterator_;
    return std::move(copy);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>&
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator--()
{
    --iterator_;
    return *this;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator--(int)
{
    IteratorBase copy(*this);
    --iterator_;
    return std::move(copy);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>&
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator+=(
    DifferenceType value)
{
    iterator_ += value;
    return *this;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>&
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator-=(
    DifferenceType value)
{
    iterator_ -= value;
    return *this;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator+(
    DifferenceType value) const
{
    IteratorBase copy(*this);
    copy += value;
    return std::move(copy);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator-(
    DifferenceType value) const
{
    IteratorBase copy(*this);
    copy -= value;
    return std::move(copy);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>::DifferenceType
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator-(
    const IteratorBase& other) const
{
    return iterator_ - other.iterator_;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator==(
    const IteratorBase& other) const
{
    return (iterator_ == other.iterator_);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator!=(
    const IteratorBase& other) const
{
    return (iterator_ != other.iterator_);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator<(
    const IteratorBase& other) const
{
    return iterator_ < other.iterator_;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator<=(
    const IteratorBase& other) const
{
    return iterator_ <= other.iterator_;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator>(
    const IteratorBase& other) const
{
    return iterator_ > other.iterator_;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
bool BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator>=(
    const IteratorBase& other) const
{
    return iterator_ >= other.iterator_;
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>::Reference
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator*()
{
    auto& constThisRef = static_cast<const IteratorBase&>(*this);
    auto& constRef = *constThisRef;
    return const_cast<Reference>(constRef);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>::ConstReference
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator*() const
{
    auto reinterpretArrayPtr = reinterpret_cast<const ValueTypeArray*>(&queue_.array_);
    auto queueArrayBegin = reinterpretArrayPtr->begin();
    auto idx = iterator_ - queueArrayBegin;
    GASSERT(0 <= idx);
    return queue_[static_cast<std::size_t>(idx)];
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>::Pointer
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator->()
{
    return &(*(*this));
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator TDerived&()
{
    return static_cast<TDerived&>(*this);
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator const TDerived&() const
{
    return static_cast<const TDerived&>(*this);
}


template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
typename BasicStaticQueueBase<T, TSize>::template IteratorBase<TDerived, TQueueType>::ConstPointer
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::operator->() const
{
    return &(*(*this));
}

template <typename T, std::size_t TSize>
template <typename TDerived, typename TQueueType>
BasicStaticQueueBase<T, TSize>::IteratorBase<TDerived, TQueueType>::IteratorBase(
    QueueType& queue,
    ArrayIterator iterator)
    : queue_(queue),
      iterator_(iterator)
{
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::ConstIterator::ConstIterator(
    QueueType& queue,
    ArrayIterator iterator)
    : Base(queue, iterator)
{
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::Iterator::Iterator(
    QueueType& queue,
    ArrayIterator iterator)
    : Base(queue, iterator)
{
}

template <typename T, std::size_t TSize>
BasicStaticQueueBase<T, TSize>::Iterator::operator ConstIterator() const
{
    return ConstIterator(Base::queue_, Base::iterator_);
}


}  // namespace container

}  // namespace embxx