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

#include <vector>
#include <cstdint>

namespace BongoJam {

    using namespace std;

    struct ResourceHandle
    {
        uint32_t Index;
        uint32_t Generation;
    };

    template<typename T>
    class DynamicMemoryArena
    {
    public:
        struct Slot
        {
            T Value;
            uint32_t Generation = 0;
            bool InUse = false;
        };

        vector<Slot> pm_Slots;

        DynamicMemoryArena(size_t fp_InitialCapacity = 64)
        {
            pm_Slots.resize(fp_InitialCapacity);
        }

        ResourceHandle 
            Allocate(T&& fp_Value) 
        {
            for (size_t i = 0; i < pm_Slots.size(); ++i)
            {
                if (not pm_Slots[i].InUse) 
                {
                    pm_Slots[i].InUse = true;
                    pm_Slots[i].Value = move(fp_Value);
                    return ResourceHandle{ static_cast<uint32_t>(i), pm_Slots[i].Generation };
                }
            }

            // Grow the arena
            size_t f_OldSize = pm_Slots.size();
            size_t f_NewSize = f_OldSize * 2;
            pm_Slots.resize(f_NewSize); // default-construct new pm_Slots

            // Use first new slot
            pm_Slots[f_OldSize].InUse = true;
            pm_Slots[f_OldSize].Value = move(fp_Value);

            return ResourceHandle{ static_cast<uint32_t>(f_OldSize), pm_Slots[f_OldSize].Generation };
        }

        T* 
            Get(const ResourceHandle& fp_ResourceHandle)
        {
            if (fp_ResourceHandle.Index >= pm_Slots.size())
            {
                return nullptr;
            }
            auto& f_Slot = pm_Slots[fp_ResourceHandle.Index];

            return (f_Slot.InUse and f_Slot.Generation == fp_ResourceHandle.Generation) ? &f_Slot.Value : nullptr;
        }

        bool 
            Free(const ResourceHandle& fp_ResourceHandle)
        {
            if (fp_ResourceHandle.Index >= pm_Slots.size())
            {
                return false;
            }

            auto& f_Slot = pm_Slots[fp_ResourceHandle.Index];

            if (not f_Slot.InUse or f_Slot.Generation != fp_ResourceHandle.Generation)
            {
                return false;
            }
            f_Slot.InUse = false;
            ++f_Slot.Generation;
            return true;
        }

        void Clear() 
        {
            for (auto& lv_Slot : pm_Slots)
            {
                lv_Slot.InUse = false;
                ++lv_Slot.Generation;
            }
        }

        size_t 
            Size() 
            const 
        { 
            return pm_Slots.size(); 
        }
        size_t 
            Used() 
            const
        {
            return count_if
            (
                pm_Slots.begin(), 
                pm_Slots.end(), 
                [](auto& s) 
                { 
                    return s.InUse; 
                }
            );
        }
    };


}//namespace BongoJam