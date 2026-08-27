//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++
// |  |  |__   |  |  | | | |  version 3.12.0
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2026 Niels Lohmann <https://nlohmann.me>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional> // equal_to, less
#include <initializer_list> // initializer_list
#include <iterator> // input_iterator_tag, iterator_traits
#include <memory> // allocator
#include <stdexcept> // for out_of_range
#include <type_traits> // enable_if, is_convertible
#include <utility> // pair
#include <vector> // vector

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/detail/meta/type_traits.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN

/// ordered_map: a minimal map-like container that preserves insertion order
/// for use within nlohmann::basic_json<ordered_map>
template <class Key, class T, class IgnoredLess = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
              struct ordered_map : std::vector<std::pair<const Key, T>, Allocator>
{
    /// the type of the keys stored in the map
    using key_type = Key;
    /// the type of the values mapped to each key
    using mapped_type = T;
    /// the underlying container type used to store the key-value pairs
    using Container = std::vector<std::pair<const Key, T>, Allocator>;
    /// an iterator over the underlying container
    using iterator = typename Container::iterator;
    /// a const iterator over the underlying container
    using const_iterator = typename Container::const_iterator;
    /// an unsigned integer type used to represent sizes and counts
    using size_type = typename Container::size_type;
    /// the type of the key-value pairs stored in the underlying container
    using value_type = typename Container::value_type;
#ifdef JSON_HAS_CPP_14
    /// the comparison function used to determine whether two keys are equal
    using key_compare = std::equal_to<>;
#else
    /// the comparison function used to determine whether two keys are equal
    using key_compare = std::equal_to<Key>;
#endif

    // Explicit constructors instead of `using Container::Container`
    // otherwise older compilers choke on it (GCC <= 5.5, xcode <= 9.4)
    /// @brief constructs an empty ordered_map
    ordered_map() noexcept(noexcept(Container())) : Container{} {}
    /// @brief constructs an empty ordered_map using the given allocator
    /// @param[in] alloc allocator to use for the underlying container
    explicit ordered_map(const Allocator& alloc) noexcept(noexcept(Container(alloc))) : Container{alloc} {}
    /// @brief constructs an ordered_map with the elements in the range [first, last)
    /// @param[in] first iterator to the first element to insert
    /// @param[in] last iterator past the last element to insert
    /// @param[in] alloc allocator to use for the underlying container
    template <class It>
    ordered_map(It first, It last, const Allocator& alloc = Allocator())
        : Container{first, last, alloc} {}
    /// @brief constructs an ordered_map from an initializer list of key-value pairs
    /// @param[in] init initializer list of key-value pairs to insert
    /// @param[in] alloc allocator to use for the underlying container
    ordered_map(std::initializer_list<value_type> init, const Allocator& alloc = Allocator() )
        : Container{init, alloc} {}
    /// @brief copy constructor
    /// @param[in] other ordered_map to copy from
    ordered_map(const ordered_map& other) = default;
    /// @brief move constructor
    /// @param[in] other ordered_map to move from
    ordered_map(ordered_map&& other) noexcept(std::is_nothrow_move_constructible<Container>::value) = default;
    /// @brief destructor
    ~ordered_map() = default;

    /// @brief copy assignment operator
    /// @param[in] other ordered_map to copy from
    /// @return reference to this ordered_map
    ordered_map& operator=(const ordered_map& other)
    {
        if (this != &other)
        {
            ordered_map tmp(other);
            Container::operator=(std::move(static_cast<Container&>(tmp)));
        }
        return *this;
    }

    /// @brief move assignment operator
    /// @param[in] other ordered_map to move from
    /// @return reference to this ordered_map
    ordered_map& operator=(ordered_map&& other) noexcept(std::is_nothrow_move_assignable<Container>::value)
    {
        Container::operator=(std::move(static_cast<Container&>(other)));
        return *this;
    }

    /// @brief inserts a new element constructed from @a key and @a t if the key does not already exist
    /// @param[in] key key of the element to insert
    /// @param[in] t value of the element to insert
    /// @return pair of an iterator to the (possibly pre-existing) element and a bool that is
    ///         true if the insertion took place
    std::pair<iterator, bool> emplace(const key_type& key, T&& t)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return {it, false};
            }
        }
        Container::emplace_back(key, std::forward<T>(t));
        return {std::prev(this->end()), true};
    }

    /// @brief inserts a new element constructed from @a key and @a t if the key does not already exist
    /// @param[in] key key of the element to insert
    /// @param[in] t value of the element to insert
    /// @return pair of an iterator to the (possibly pre-existing) element and a bool that is
    ///         true if the insertion took place
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    std::pair<iterator, bool> emplace(KeyType && key, T && t)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return {it, false};
            }
        }
        Container::emplace_back(std::forward<KeyType>(key), std::forward<T>(t));
        return {std::prev(this->end()), true};
    }

    /// @brief accesses the element with the given key, inserting a default-constructed value if it does not exist
    /// @param[in] key key of the element to access
    /// @return reference to the value associated with @a key
    T& operator[](const key_type& key)
    {
        return emplace(key, T{}).first->second;
    }

    /// @brief accesses the element with the given key, inserting a default-constructed value if it does not exist
    /// @param[in] key key of the element to access
    /// @return reference to the value associated with @a key
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    T & operator[](KeyType && key)
    {
        return emplace(std::forward<KeyType>(key), T{}).first->second;
    }

    /// @brief accesses the element with the given key
    /// @param[in] key key of the element to access
    /// @return const reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    const T& operator[](const key_type& key) const
    {
        return at(key);
    }

    /// @brief accesses the element with the given key
    /// @param[in] key key of the element to access
    /// @return const reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    const T & operator[](KeyType && key) const
    {
        return at(std::forward<KeyType>(key));
    }

    /// @brief accesses the element with the given key, with bounds checking
    /// @param[in] key key of the element to access
    /// @return reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    T& at(const key_type& key)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it->second;
            }
        }

        JSON_THROW(std::out_of_range("key not found"));
    }

    /// @brief accesses the element with the given key, with bounds checking
    /// @param[in] key key of the element to access
    /// @return reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    T & at(KeyType && key) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it->second;
            }
        }

        JSON_THROW(std::out_of_range("key not found"));
    }

    /// @brief accesses the element with the given key, with bounds checking
    /// @param[in] key key of the element to access
    /// @return const reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    const T& at(const key_type& key) const
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it->second;
            }
        }

        JSON_THROW(std::out_of_range("key not found"));
    }

    /// @brief accesses the element with the given key, with bounds checking
    /// @param[in] key key of the element to access
    /// @return const reference to the value associated with @a key
    /// @throws std::out_of_range if the container does not have an element with the given key
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    const T & at(KeyType && key) const // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it->second;
            }
        }

        JSON_THROW(std::out_of_range("key not found"));
    }

    /// @brief removes the element with the given key, if it exists
    /// @param[in] key key of the element to remove
    /// @return number of elements removed (0 or 1)
    size_type erase(const key_type& key)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                // Since we cannot move const Keys, re-construct them in place
                for (auto next = it; ++next != this->end(); ++it)
                {
                    it->~value_type(); // Destroy but keep allocation
                    new (&*it) value_type{std::move(*next)};
                }
                Container::pop_back();
                return 1;
            }
        }
        return 0;
    }

    /// @brief removes the element with the given key, if it exists
    /// @param[in] key key of the element to remove
    /// @return number of elements removed (0 or 1)
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    size_type erase(KeyType && key) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                // Since we cannot move const Keys, re-construct them in place
                for (auto next = it; ++next != this->end(); ++it)
                {
                    it->~value_type(); // Destroy but keep allocation
                    new (&*it) value_type{std::move(*next)};
                }
                Container::pop_back();
                return 1;
            }
        }
        return 0;
    }

    /// @brief removes the element at the given position
    /// @param[in] pos iterator to the element to remove
    /// @return iterator following the removed element
    iterator erase(iterator pos)
    {
        return erase(pos, std::next(pos));
    }

    /// @brief removes the elements in the range [first, last)
    /// @param[in] first iterator to the first element to remove
    /// @param[in] last iterator past the last element to remove
    /// @return iterator following the last removed element
    iterator erase(iterator first, iterator last)
    {
        if (first == last)
        {
            return first;
        }

        const auto elements_affected = std::distance(first, last);
        const auto offset = std::distance(Container::begin(), first);

        // This is the start situation. We need to delete elements_affected
        // elements (3 in this example: e, f, g), and need to return an
        // iterator past the last deleted element (h in this example).
        // Note that offset is the distance from the start of the vector
        // to first. We will need this later.

        // [ a, b, c, d, e, f, g, h, i, j ]
        //               ^        ^
        //             first    last

        // Since we cannot move const Keys, we re-construct them in place.
        // We start at first and re-construct (viz. copy) the elements from
        // the back of the vector. Example for the first iteration:

        //               ,--------.
        //               v        |   destroy e and re-construct with h
        // [ a, b, c, d, e, f, g, h, i, j ]
        //               ^        ^
        //               it       it + elements_affected

        for (auto it = first; std::next(it, elements_affected) != Container::end(); ++it)
        {
            it->~value_type(); // destroy but keep allocation
            new (&*it) value_type{std::move(*std::next(it, elements_affected))}; // "move" next element to it
        }

        // [ a, b, c, d, h, i, j, h, i, j ]
        //               ^        ^
        //             first    last

        // remove the unneeded elements at the end of the vector
        Container::resize(this->size() - static_cast<size_type>(elements_affected));

        // [ a, b, c, d, h, i, j ]
        //               ^        ^
        //             first    last

        // first is now pointing past the last deleted element, but we cannot
        // use this iterator, because it may have been invalidated by the
        // resize call. Instead, we can return begin() + offset.
        return Container::begin() + offset;
    }

    /// @brief returns the number of elements matching the given key
    /// @param[in] key key to count
    /// @return 1 if an element with the key exists, 0 otherwise
    size_type count(const key_type& key) const
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return 1;
            }
        }
        return 0;
    }

    /// @brief returns the number of elements matching the given key
    /// @param[in] key key to count
    /// @return 1 if an element with the key exists, 0 otherwise
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    size_type count(KeyType && key) const // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return 1;
            }
        }
        return 0;
    }

    /// @brief finds an element with the given key
    /// @param[in] key key to search for
    /// @return iterator to the element if found, or end() otherwise
    iterator find(const key_type& key)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it;
            }
        }
        return Container::end();
    }

    /// @brief finds an element with the given key
    /// @param[in] key key to search for
    /// @return iterator to the element if found, or end() otherwise
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    iterator find(KeyType && key) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it;
            }
        }
        return Container::end();
    }

    /// @brief finds an element with the given key
    /// @param[in] key key to search for
    /// @return const iterator to the element if found, or end() otherwise
    const_iterator find(const key_type& key) const
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it;
            }
        }
        return Container::end();
    }

    /// @brief finds an element with the given key
    /// @param[in] key key to search for
    /// @return const iterator to the element if found, or end() otherwise
    template<class KeyType, detail::enable_if_t<
                 detail::is_usable_as_key_type<key_compare, key_type, KeyType>::value, int> = 0>
    const_iterator find(KeyType && key) const // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, key))
            {
                return it;
            }
        }
        return Container::end();
    }

    /// @brief inserts an element if the key does not already exist
    /// @param[in] value key-value pair to insert
    /// @return pair of an iterator to the (possibly pre-existing) element and a bool that is
    ///         true if the insertion took place
    std::pair<iterator, bool> insert( value_type&& value )
    {
        return emplace(value.first, std::move(value.second));
    }

    /// @brief inserts an element if the key does not already exist
    /// @param[in] value key-value pair to insert
    /// @return pair of an iterator to the (possibly pre-existing) element and a bool that is
    ///         true if the insertion took place
    std::pair<iterator, bool> insert( const value_type& value )
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            if (m_compare(it->first, value.first))
            {
                return {it, false};
            }
        }
        Container::push_back(value);
        return {--this->end(), true};
    }

    /// helper type that is only defined if @a InputIt qualifies as an input iterator,
    /// used to constrain the iterator-range overload of @ref insert
    template<typename InputIt>
    using require_input_iter = typename std::enable_if<std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category,
        std::input_iterator_tag>::value>::type;

    /// @brief inserts the elements in the range [first, last), skipping keys that already exist
    /// @param[in] first iterator to the first element to insert
    /// @param[in] last iterator past the last element to insert
    template<typename InputIt, typename = require_input_iter<InputIt>>
    void insert(InputIt first, InputIt last)
    {
        for (auto it = first; it != last; ++it)
        {
            insert(*it);
        }
    }

private:
    JSON_NO_UNIQUE_ADDRESS key_compare m_compare = key_compare();
};

NLOHMANN_JSON_NAMESPACE_END
