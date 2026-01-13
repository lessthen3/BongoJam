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

///BongoJam
#include "../Logger.h"

///STL
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <new>

namespace BongoJam {

    struct BumpAllocator
    {
    private:
        uint8_t* pm_Buffer = nullptr;
        size_t pm_Capacity = 0;
        size_t pm_Offset = 0;

    public:
        BumpAllocator(size_t bytes)
        {
            pm_Buffer = static_cast<uint8_t*>(malloc(bytes));

            if (not pm_Buffer) 
            {
                throw bad_alloc();
            }

            pm_Capacity = bytes;
            pm_Offset = 0;
        }

        ~BumpAllocator()
        {
            free(pm_Buffer);
            pm_Buffer = nullptr;
            pm_Capacity = 0;
            pm_Offset = 0;
        }

        void* 
            Allocate(size_t size, size_t alignment = alignof(max_align_t))
        {
            size_t f_AlignedOffset = (pm_Offset + alignment - 1) & ~(alignment - 1);

            if (f_AlignedOffset + size > pm_Capacity)
            {
                throw bad_alloc();
            }

            void* f_Ptr = pm_Buffer + f_AlignedOffset;
            pm_Offset = f_AlignedOffset + size;

            return f_Ptr;
        }

        template<typename T, typename... Args>
        T* 
            Construct(Args&&... args)
        {
            void* f_Ptr = Allocate(sizeof(T), alignof(T));
            return new (f_Ptr) T(forward<Args>(args)...);
        }

        void 
            Reset()
        {
            pm_Offset = 0;
        }

        size_t Used() const { return pm_Offset; }
        size_t Remaining() const { return pm_Capacity - pm_Offset; }
        size_t Capacity() const { return pm_Capacity; }
    };

} //namespace BongoJam