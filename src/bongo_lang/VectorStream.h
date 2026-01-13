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

namespace BongoJam
{
    using namespace std;

    //////////////////////////////////////////////
    // Helper Struct for Parsing
    //////////////////////////////////////////////

    template<typename T>
    struct VectorStream
    {
        explicit
            VectorStream(vector<T>&& fp_Vector) : pm_Vector(move(fp_Vector)) {}

        [[nodiscard]] bool
            IsEmpty()
            const noexcept
        {
            return pm_Position >= pm_Vector.size();
        }

        [[nodiscard]] bool
            ShiftForward(T& fp_Out)
        {
            if (IsEmpty())
            {
                return false;
            }

            fp_Out = pm_Vector[pm_Position++];

            return true;
        }

        void
            ShiftForwardUnsafe(T& fp_Out)
        {
            fp_Out = pm_Vector[pm_Position++];
        }

        [[nodiscard]] bool
            Peek(T& fp_Out)
            const
        {
            if (IsEmpty())
            {
                return false;
            }

            fp_Out = pm_Vector[pm_Position];

            return true;
        }

        [[nodiscard]] bool
            Peek(size_t fp_Index, T& fp_Out)
            const
        {
            if (fp_Index == 0)   // invalid by definition, if 0 size_t underflows after decrement
            {
                return false;
            }

            --fp_Index; //need decrement since the current index is the next element since ShiftForward only allows catching the current index

            if (pm_Position + fp_Index >= pm_Vector.size())
            {
                return false;
            }

            fp_Out = pm_Vector[pm_Position + fp_Index];

            return true;
        }

    private:
        const vector<T> pm_Vector;
        size_t pm_Position = 0;
    };
}
