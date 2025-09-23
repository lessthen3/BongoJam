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

#include <array>
#include <cstdint>

namespace BongoJam {

    using namespace std;

    template<typename T, size_t Capacity>
    struct MemoryArena
    {
    public:
        struct Slot 
        {
            T value;
            uint32_t generation = 0;
            bool in_use = false;
        };

        array<Slot, Capacity> slots;

        ResourceHandle allocate(const T& value) 
        {
            for (size_t i = 0; i < Capacity; ++i) 
            {
                if (!slots[i].in_use) 
                {
                    slots[i].in_use = true;
                    slots[i].value = value;
                    return ResourceHandle{ i, slots[i].generation };
                }
            }
            throw runtime_error("Arena full");
        }

        T* get(const ResourceHandle& handle)
        {
            auto& slot = slots[handle.index];

            if (!slot.in_use || slot.generation != handle.generation)
            {
                return nullptr;
            }

            return &slot.value;
        }

        void free(const ResourceHandle& handle)
        {
            auto& slot = slots[handle.index];
            slot.in_use = false;
            ++slot.generation;
        }
    };

}//namespace BongoJam