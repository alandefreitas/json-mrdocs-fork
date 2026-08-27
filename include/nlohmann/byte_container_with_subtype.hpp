//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++
// |  |  |__   |  |  | | | |  version 3.12.0
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2026 Niels Lohmann <https://nlohmann.me>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint> // uint8_t, uint64_t
#include <tuple> // tie
#include <utility> // move

#include <nlohmann/detail/abi_macros.hpp>

/// @brief namespace for Niels Lohmann
/// @sa https://github.com/nlohmann
NLOHMANN_JSON_NAMESPACE_BEGIN

/// @brief an internal type for a backed binary type
/// @sa https://json.nlohmann.me/api/byte_container_with_subtype/
template<typename BinaryType>
class byte_container_with_subtype : public BinaryType
{
  public:
    /// @brief the type of the underlying container storing the binary data
    using container_type = BinaryType;
    /// @brief the type used to store the binary subtype
    using subtype_type = std::uint64_t;

    /// @brief creates an empty binary value with no subtype
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/byte_container_with_subtype/
    byte_container_with_subtype() noexcept(noexcept(container_type()))
        : container_type()
    {}

    /// @brief creates a binary value with no subtype from a copy of the underlying container
    /// @param b the container data to copy into the binary value
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/byte_container_with_subtype/
    byte_container_with_subtype(const container_type& b) noexcept(noexcept(container_type(b)))
        : container_type(b)
    {}

    /// @brief creates a binary value with no subtype by moving from the underlying container
    /// @param b the container data to move into the binary value
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/byte_container_with_subtype/
    byte_container_with_subtype(container_type&& b) noexcept(noexcept(container_type(std::move(b))))
        : container_type(std::move(b))
    {}

    /// @brief creates a binary value with the given subtype from a copy of the underlying container
    /// @param b the container data to copy into the binary value
    /// @param subtype_ the subtype to assign to the binary value
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/byte_container_with_subtype/
    byte_container_with_subtype(const container_type& b, subtype_type subtype_) noexcept(noexcept(container_type(b)))
        : container_type(b)
        , m_subtype(subtype_)
        , m_has_subtype(true)
    {}

    /// @brief creates a binary value with the given subtype by moving from the underlying container
    /// @param b the container data to move into the binary value
    /// @param subtype_ the subtype to assign to the binary value
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/byte_container_with_subtype/
    byte_container_with_subtype(container_type&& b, subtype_type subtype_) noexcept(noexcept(container_type(std::move(b))))
        : container_type(std::move(b))
        , m_subtype(subtype_)
        , m_has_subtype(true)
    {}

    /// @brief compares this value to @a rhs for equality, including the subtype
    /// @param rhs the value to compare against
    /// @return whether the underlying container and subtype are equal to those of @a rhs
    bool operator==(const byte_container_with_subtype& rhs) const
    {
        return std::tie(static_cast<const BinaryType&>(*this), m_subtype, m_has_subtype) ==
               std::tie(static_cast<const BinaryType&>(rhs), rhs.m_subtype, rhs.m_has_subtype);
    }

    /// @brief compares this value to @a rhs for inequality, including the subtype
    /// @param rhs the value to compare against
    /// @return whether the underlying container or subtype differ from those of @a rhs
    bool operator!=(const byte_container_with_subtype& rhs) const
    {
        return !(rhs == *this);
    }

    /// @brief sets the binary subtype
    /// @param subtype_ the subtype to set
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/set_subtype/
    void set_subtype(subtype_type subtype_) noexcept
    {
        m_subtype = subtype_;
        m_has_subtype = true;
    }

    /// @brief return the binary subtype
    /// @return the binary subtype, or -1 if the value has no subtype
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/subtype/
    constexpr subtype_type subtype() const noexcept
    {
        return m_has_subtype ? m_subtype : static_cast<subtype_type>(-1);
    }

    /// @brief return whether the value has a subtype
    /// @return whether the value has a subtype
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/has_subtype/
    constexpr bool has_subtype() const noexcept
    {
        return m_has_subtype;
    }

    /// @brief clears the binary subtype
    /// @sa https://json.nlohmann.me/api/byte_container_with_subtype/clear_subtype/
    void clear_subtype() noexcept
    {
        m_subtype = 0;
        m_has_subtype = false;
    }

  private:
    subtype_type m_subtype = 0;
    bool m_has_subtype = false;
};

NLOHMANN_JSON_NAMESPACE_END
