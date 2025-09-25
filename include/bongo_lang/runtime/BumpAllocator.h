/**************************************************************************
 *                         BongoJam Script v0.0.1
 *                  Created by Ranyodh Mandur - 🗻2024
 *
 *                 Licensed under the MIT License (MIT).
 *            For more details, see the LICENSE file or visit:
 *                 https://opensource.org/licenses/MIT
 *
 * BongoJam is an open-source scripting language compiler and interpreter
 *        primarily intended for embedding within game engines.
**************************************************************************/
#pragma once

///STL
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <new>

namespace BongoJam {

    using namespace std;

    struct BumpAllocator
    {
    private:
        uint8_t* m_Buffer = nullptr;
        size_t m_Capacity = 0;
        size_t m_Offset = 0;

    public:
        BumpAllocator(size_t bytes)
        {
            m_Buffer = static_cast<uint8_t*>(malloc(bytes));
            if (not m_Buffer) throw bad_alloc();
            m_Capacity = bytes;
            m_Offset = 0;
        }

        ~BumpAllocator()
        {
            free(m_Buffer);
            m_Buffer = nullptr;
            m_Capacity = 0;
            m_Offset = 0;
        }

        void* 
            Allocate(size_t size, size_t alignment = alignof(max_align_t))
        {
            size_t alignedOffset = (m_Offset + alignment - 1) & ~(alignment - 1);
            if (alignedOffset + size > m_Capacity)
            {
                throw bad_alloc();
            }

            void* ptr = m_Buffer + alignedOffset;
            m_Offset = alignedOffset + size;
            return ptr;
        }

        template<typename T, typename... Args>
        T* 
            Construct(Args&&... args)
        {
            void* ptr = Allocate(sizeof(T), alignof(T));
            return new (ptr) T(forward<Args>(args)...);
        }

        void 
            Reset()
        {
            m_Offset = 0;
        }

        size_t Used() const { return m_Offset; }
        size_t Remaining() const { return m_Capacity - m_Offset; }
        size_t Capacity() const { return m_Capacity; }
    };

} //namespace BongoJam