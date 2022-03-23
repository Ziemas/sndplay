// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once

#include "types.h"
#include <queue>
#include <unordered_map>

namespace snd {

template <typename type>
class handler_map {
public:
    using iterator = typename std::unordered_map<u32, type>::iterator;

    template <class... Args>
    u32 emplace(Args&&... args)
    {
        u32 id = get_id();
        m_map.emplace(id, std::forward<Args>(args)...);
        return id;
    }

    u32 emplace(type item)
    {
        u32 id = get_id();
        m_map.emplace(id, item);
        return id;
    }

    size_t size() { return m_map.size(); }

    iterator begin() { return m_map.begin(); }
    iterator end() { return m_map.end(); }
    iterator erase(iterator it)
    {
        free_id(it->first);
        return m_map.erase(it);
    };

    type& at(const u32& key)
    {
        return m_map.at(key);
    }

private:
    u32 get_id()
    {
        u32 id = 0;
        if (m_free_ids.empty()) {
            id = next_id++;
        } else {
            id = m_free_ids.front();
            m_free_ids.pop();
        }
        return id;
    }

    void free_id(u32 id)
    {
        m_free_ids.push(id);
    }

    u32 next_id { 1 };
    std::queue<u32> m_free_ids;
    std::unordered_map<u32, type> m_map;
};

class sound_handler {
public:
    virtual ~sound_handler() = default;
    virtual bool tick() = 0;

private:
};
}
