/**
 * @file memory_pool.h
 * @author lzc (liuzechen.coder@qq.com)
 * @brief 内存池
 * @version 0.1
 * @date 2024-02-27
 * @note https://github.com/cacay/MemoryPool
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MY_CODE_MEMORY_H
#define MY_CODE_MEMORY_H

#include <cstddef>
#include <cstdint>
#include <utility>

namespace my_code {
template <typename T, size_t BlockSize = 4096>
class MemoryPool {
    MemoryPool() noexcept = default;
    // wtf
    MemoryPool(const MemoryPool& memory_pool) noexcept : MemoryPool() {}

    // 这里感觉可以直接禁用掉
    MemoryPool(MemoryPool&& rhs) noexcept {
        m_current_block = std::exchange(rhs.m_current_block, nullptr);
        m_current_slot = rhs.m_current_slot;
        m_last_slot = rhs.m_last_slot;
        m_free_slots = rhs.m_free_slots;
    }

    template <typename U>
    MemoryPool(const MemoryPool<U>& memory_pool) noexcept : MemoryPool() {}

    ~MemoryPool() noexcept {
        Slot* cur = m_current_block;
        while (cur != nullptr) {
            Slot* next = cur->next;
            operator delete(reinterpret_cast<void*>(cur));
            cur = next;
        }
    }

    MemoryPool& operator=(const MemoryPool&) = delete;

    MemoryPool& operator=(MemoryPool&& rhs) {
        if (this != &rhs) {
            std::swap(m_current_block, rhs.m_current_block);
            m_current_slot = rhs.m_current_slot;
            m_last_slot = rhs.m_last_slot;
            m_free_slots = rhs.m_free_slots;
        }
        return *this;
    }

    T* address(T& val) const noexcept { return &val; }
    const T* address(const T& val) const noexcept { return &val; }

    T* alloc(size_t n = 1, const T* hint = 0) {
        // 如果说能分配完，那么就直接分配
        if (m_free_slots != nullptr) {
            T* result = reinterpret_cast<T*>(m_free_slots);
            m_free_slots = m_free_slots->next;
            return result;
        } else {
            if (m_current_slot >= m_last_slot) {
                allocate_block();
                return reinterpret_cast<T*>(m_current_slot++);
            }
        }
    }

    void dealloc(T* ptr, size_t n = 1) {
        if (ptr != nullptr) {
            // 链子炼起来
            reinterpret_cast<Slot*>(ptr)->next = m_free_slots;
            m_free_slots = reinterpret_cast<Slot*>(ptr);
        }
    }

    size_t max_size() const noexcept {
        size_t max_blocks = -1 / BlockSize;
        // wtf ??
        return (BlockSize - sizeof(char*)) / sizeof(Slot) * max_blocks;
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        ::new (ptr) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

    template <typename... Args>
    T* new_element(Args&&... args) {
        T* result = alloc();
        construct<T>(result, std::forward<Args>(args)...);
        return result;
    }

    template <typename... Args>
    void delete_element(T* ptr) {
        if (ptr != nullptr) {
            ptr->~T();
            dealloc(ptr);
        }
    }

private:
    union Slot {
        T element;
        Slot* next;
    };

    size_t pad_pointer(char* ptr, size_t align) const noexcept {
        uintptr_t result = reinterpret_cast<uintptr_t>(ptr);
        return ((align - result) % align);
    }

    void allocate_block() {
        char* new_block = reinterpret_cast<char*>(operator new(BlockSize));
        reinterpret_cast<Slot*>(new_block)->next = m_current_block;
        m_current_block = reinterpret_cast<Slot*>(new_block);
        char* body = new_block + sizeof(Slot*);
        size_t body_padding = pad_pointer(body, alignof(Slot));
        m_current_slot = reinterpret_cast<Slot>(body + body_padding);
        m_last_slot = reinterpret_cast<Slot*>(new_block + BlockSize - sizeof(Slot) + 1);
    }

private:
    Slot* m_current_block { nullptr };
    Slot* m_current_slot { nullptr };
    Slot* m_last_slot { nullptr };
    Slot* m_free_slots { nullptr };
    static_assert(BlockSize >= 2 * sizeof(Slot), "BlockSize too samll");
};
} // namespace my_code
#endif