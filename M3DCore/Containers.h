// SnaX Game Engine - https://github.com/snaxgameengine/snax
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <array>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "Allocator.h"

namespace m3d
{

template<typename T, std::size_t SIZE>
using Array = std::array<T, SIZE>;

template<typename T, typename A = Allocator<T>>
using List = std::vector<T, A>;

template<typename T, typename CMP = std::less<T>, typename A = Allocator<T>>
using Set = std::set<T, CMP, A>;

template<typename T, typename CMP = std::less<T>, typename A = Allocator<T>>
using MultiSet = std::multiset<T, CMP, A>;

template<typename T, typename S, typename CMP = std::less<T>, typename A = Allocator<std::pair<const T, S>>>
using Map = std::map<T, S, CMP, A>;

template<typename T, typename S, typename CMP = std::less<T>, typename A = Allocator<std::pair<const T, S>>>
using MultiMap = std::multimap<T, S, CMP, A>;

template<typename T, typename S, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = Allocator<std::pair<const T, S>>>
using UnorderedMap = std::unordered_map<T, S, H, E, A>;

template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = Allocator<T>>
using UnorderedSet = std::unordered_set<T, H, E, A>;




template <template<typename> typename H = std::hash, typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    seed ^= H<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine<H>(seed, rest), ...);
}

template <template<typename> typename H = std::hash, typename... V>
std::size_t hash_combine_simple(V... v)
{
    std::size_t seed = 0;
    auto f = [&seed](size_t s) { seed ^= s + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
    (f(H<V>()(v)), ...);
    return seed;
}


template<typename T, typename V = void>
struct Hash
{
    std::size_t operator()(T const& t) const noexcept
    {
        return std::hash<T>()(t);
    }
};

template<typename T>
struct Hash<T, typename std::enable_if<std::is_class<T>::value>::type>
{
    std::size_t operator()(T const& t) const noexcept
    {
        return t.hash();
    }
};

template<typename T, std::size_t SIZE>
struct Hash<std::array<T, SIZE>, typename std::enable_if<std::is_class<std::array<T, SIZE>>::value>::type>
{
    std::size_t operator()(std::array<T, SIZE> const& s) const noexcept
    {
        std::size_t h = 0;
        for (const T& n : s)
            hash_combine<m3d::Hash>(h, n);
        return h;
    }
};

template<typename T, typename A>
struct Hash<std::vector<T, A>, typename std::enable_if<std::is_class<std::vector<T, A>>::value>::type>
{
    std::size_t operator()(std::vector<T, A> const& s) const noexcept
    {
        std::size_t h = 0;
        for (const T& n : s)
            hash_combine<m3d::Hash>(h, n);
        return h;
    }
};

template<typename ...Args>
struct Hash<std::tuple<Args...>, typename std::enable_if<std::is_class<std::tuple<Args...>>::value>::type>
{
    std::size_t operator()(std::tuple<Args...> const& s) const noexcept
    {
        std::size_t h = 0;
        std::apply([&h](auto&&... args) {((hash_combine<m3d::Hash>(h, args)), ...); }, s);
        return h;
    }
};

}