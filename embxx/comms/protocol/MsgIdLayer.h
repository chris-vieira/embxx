//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
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

/// @file embxx/comms/protocol/MsgIdLayer.h
/// This file contains "Message ID" protocol layer of the "comms" module.

#pragma once

#include <array>
#include <tuple>
#include <algorithm>
#include <streambuf>

#include "embxx/util/Assert.h"
#include "embxx/util/Tuple.h"
#include "embxx/comms/traits.h"
#include "ProtocolLayer.h"

namespace embxx
{

namespace comms
{

namespace protocol
{

/// @ingroup comms
/// @brief Protocol layer that uses message ID to differentiate between messages.
/// @details This layers is a "must have" one, it contains allocator to allocate
///          message object.
/// @tparam TMsgBase Base class for all the custom messages, smart pointer to
///         which will be returned from read() member function.
/// @tparam TAllMessages A tuple (std::tuple) of all the custom message types
///         this protocol layer must support.
/// @tparam TAllocator The allocator class, will be used to allocate message
///         objects in read() member function.
///         The requirements for the allocator are:
///         @li Must have a default (no arguments) constructor.
///         @li Must provide allocation function to allocate message
///             objects. The signature must be as following:
///             @code template <typename TObj, typename... TArgs> std::unique_ptr<TObj, Deleter> alloc(TArgs&&... args); @endcode
///             The Deleter maybe either default "std::default_delete<T>" or
///             custom one. All the allocators defined in "util" module
///             (header: "embxx/util/Allocators.h") satisfy these requirements.
///             See also embxx::comms::DynMemMsgAllocator and
///             embxx::comms::InPlaceMsgAllocator
/// @tparam TTraits A traits class that must define:
///         @li Endianness type. Either embxx::comms::traits::endian::Big or
///             embxx::comms::traits::endian::Little
///         @li MsgIdLen static integral constant specifying length of
///             message ID field in bytes.
/// @pre TMsgBase must be a base class to all custom message types bundled in
///      TAllMessages
/// @pre TAllMessages must be any variation of std::tuple
/// @headerfile embxx/comms/protocol/MsgIdLayer.h
template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
class MsgIdLayer : public ProtocolLayer<TTraits, TNextLayer>
{
    static_assert(util::IsTuple<TAllMessages>::Value,
        "TAllMessages must be of std::tuple type");
    typedef ProtocolLayer<TTraits, TNextLayer> Base;

    typedef decltype(TAllocator().template alloc<typename Base::MsgBase>()) InternalMsgPtr;

public:

    /// @brief Base class for all the messages
    typedef typename Base::MsgBase MsgBase;

    /// @brief Definition of all messages type. Must be std::tuple
    typedef TAllMessages AllMessages;

    /// @brief Definition of the allocator type
    typedef TAllocator Allocator;

    /// @brief Used traits
    typedef typename Base::Traits Traits;

    /// @brief Smart pointer to the created message.
    /// @details Equivalent to:
    ///          @code
    ///          typedef decltype(Allocator().template alloc<MsgBase>()) MsgPtr;
    ///          @endcode
    typedef InternalMsgPtr MsgPtr;

    /// Type of message ID
    typedef traits::MsgIdType MsgIdType;

    /// Length of the message ID field. Originally defined in traits
    static const std::size_t MsgIdLen = Traits::MsgIdLen;

    /// @brief Constructor
    /// @details Defines static factories responsible for generation of
    ///          custom message objects.
    /// @note Thread safety: Safe if compiler supports safe initialisation
    ///       of static data
    /// @note Exception guarantee: Basic
    MsgIdLayer();

    /// @brief Destructor
    ~MsgIdLayer();

    /// @brief Deserialise message from the data in the input stream buffer.
    /// @details The function will read message ID from the stream first,
    ///          generate appropriate message object based on the read ID and
    ///          forward the request to the next layer.
    /// @param[in, out] msgPtr Reference to smart pointer that will hold
    ///                 allocated message object
    /// @param[in, out] buf Input stream buffer
    /// @param[in] size Size of the data in the buffer
    /// @return Error status of the operation.
    /// @pre msgPtr doesn't point to any object:
    ///      @code assert(!msgPtr); @endcode
    /// @pre Value of provided "size" must be less than or equal to
    ///      available data in the buffer (size <= buf.in_avail());
    /// @post The internal (std::ios_base::in) pointer of the stream buffer
    ///       will be advanced by the number of bytes was actually read.
    ///       In case of an error, it will provide an information to the caller
    ///       about the place the error was recognised.
    /// @post Returns embxx::comms::ErrorStatus::Success if and only if msgPtr points
    ///       to a valid object.
    /// @note Thread safety: Safe on distinct MsgIdLayer object and distinct
    ///       buffers, unsafe otherwise.
    /// @note Exception guarantee: Basic
    ErrorStatus read(MsgPtr& msgPtr, std::streambuf& buf, std::size_t size);

    /// @brief Serialise message into the stream buffer.
    /// @details The function will write ID of the message to the stream
    ///          buffer, then call write() member function of the next
    ///          protocol layer.
    /// @param[in] msg Reference to message object
    /// @param[in, out] buf Output stream buffer.
    /// @param[in] size size of the buffer
    /// @return Status of the write operation.
    /// @pre Value of provided "size" must be less than or equal to
    ///      available space in the buffer.
    /// @post The internal (std::ios_base::out) pointer of the stream buffer
    ///       will be advanced by the number of bytes was actually written.
    ///       In case of an error, it will provide an information to the caller
    ///       about the place the error was recognised.
    /// @note Thread safety: Safe on distinct stream buffers, unsafe otherwise.
    /// @note Exception guarantee: Basic
    ErrorStatus write(
        const MsgBase& msg,
        std::streambuf& buf,
        std::size_t size) const;

    /// @brief Get allocator.
    /// @details Returns reference to the message allocator. It can be used
    ///          to extend initialisation of the latter if its default
    ///          constructor wasn't enough.
    /// @return Reference to message allocator
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    Allocator& getAllocator();

    /// @brief Const version of getAllocator()
    const Allocator& getAllocator() const;

private:

    class Factory
    {
    public:

        virtual ~Factory();

        MsgIdType getId() const;

        MsgPtr create(Allocator& allocator) const;

    protected:
        virtual MsgIdType getIdImpl() const = 0;
        virtual MsgPtr createImpl(Allocator& allocator) const = 0;
    };

    template <typename TMessage>
    class MsgFactory : public Factory
    {
    public:
        typedef TMessage Message;
        static const MsgIdType MsgId = Message::MsgId;
    protected:
        virtual MsgIdType getIdImpl() const;
        virtual MsgPtr createImpl(Allocator& allocator) const;
    private:
    };

    typedef std::array<Factory*, std::tuple_size<AllMessages>::value> Factories;

    Allocator allocator_;
    Factories factories_;

};

// Implementation

namespace details
{

template <std::size_t TSize>
struct FactoryCreator
{
    template <typename TAllMessages,
              template <class> class TFactory,
              typename TFactories>
    static void create(TFactories& factories)
    {
        static const std::size_t Idx = TSize - 1;
        FactoryCreator<Idx>::template
                            create<TAllMessages, TFactory>(factories);

        typedef typename std::tuple_element<Idx, TAllMessages>::type Message;
        static TFactory<Message> factory;
        factories[Idx] = &factory;
    }
};

template <>
struct FactoryCreator<0>
{
    template <typename TAllMessages,
              template <class> class TFactory,
              typename TFactories>
    static void create(TFactories& factories)
    {
        static_cast<void>(factories);
    }
};


}  // namespace details


template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgIdLayer()
{

    static const std::size_t NumOfMsgs = std::tuple_size<AllMessages>::value;
    details::FactoryCreator<NumOfMsgs>::template
                    create<AllMessages, MsgFactory>(factories_);

    std::sort(factories_.begin(), factories_.end(),
        [](Factory* factory1, Factory* factory2) -> bool
        {
            return factory1->getId() < factory2->getId();
        });
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::~MsgIdLayer()
{
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
ErrorStatus MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::read(
    MsgPtr& msgPtr,
    std::streambuf& buf,
    std::size_t size)
{
    GASSERT(!msgPtr);
    GASSERT(size <= static_cast<decltype(size)>(buf.in_avail()));

    if (size < MsgIdLen) {
        return ErrorStatus::NotEnoughData;
    }

    auto id = Base::template getData<MsgIdType, MsgIdLen>(buf);
    auto iter = std::lower_bound(factories_.begin(), factories_.end(), id,
        [](Factory* factory, MsgIdType id) -> bool
        {
            return factory->getId() < id;
        });

    if ((iter == factories_.end()) || ((*iter)->getId() != id)) {
        return ErrorStatus::InvalidMsgId;
    }

    msgPtr = (*iter)->create(allocator_);
    if (!msgPtr) {
        return ErrorStatus::MsgAllocFaulure;
    }

    auto status = Base::nextLayer().read(msgPtr, buf, size - MsgIdLen);
    if (status != ErrorStatus::Success) {
        msgPtr.reset();
    }

    return status;
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
ErrorStatus MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::write(
    const MsgBase& msg,
    std::streambuf& buf,
    std::size_t size) const
{
#ifndef NDEBUG
    auto firstPos = buf.pubseekoff(0, std::ios_base::cur, std::ios_base::out);
    auto lastPos = buf.pubseekoff(0, std::ios_base::end, std::ios_base::out);
    buf.pubseekpos(firstPos, std::ios_base::out);
    auto diff = static_cast<decltype(size)>(lastPos - firstPos);
    GASSERT(size <= diff);
#endif // #ifndef NDEBUG

    if (size < MsgIdLen) {
        return ErrorStatus::BufferOverflow;
    }

    Base::template putData<MsgIdLen>(msg.getId(), buf);
    return Base::nextLayer().write(msg, buf, size - MsgIdLen);
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::Allocator&
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::getAllocator()
{
    return allocator_;
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
const typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::Allocator&
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::getAllocator() const
{
    return allocator_;
}


template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::Factory::~Factory()
{
}


template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgIdType
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::Factory::getId() const
{
    return this->getIdImpl();
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgPtr
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::Factory::create(
    Allocator& allocator) const
{
    return this->createImpl(allocator);
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
template <typename TMessage>
typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgIdType
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgFactory<TMessage>::getIdImpl() const
{
    return MsgId;
}

template <typename TAllMessages,
          typename TAllocator,
          typename TTraits,
          typename TNextLayer>
template <typename TMessage>
typename MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgPtr
MsgIdLayer<TAllMessages, TAllocator, TTraits, TNextLayer>::MsgFactory<TMessage>::createImpl(
    Allocator& allocator) const
{
    return std::move(allocator.template alloc<Message>());
}

}  // namespace protocol

}  // namespace comms

}  // namespace embxx
