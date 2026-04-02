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
#include "Linker.h"

#include "../ErrorCodes.h"

namespace BongoJam{

    bool
        BongoLinker::ResolveSymbol()
    {

        return true;
    }

    int
        BongoLinker::LinkCompilationUnits(vector<unique_ptr<SSA::CompilationUnit>>&& fp_CompiledUnits, vector<uint8_t>& fp_FinalByteCode)
    {
        {
            int f_CurrentOffset = -1; //start at -1 for the list index offset starts at 0 , so eg if the file was one bytecode then it would index at 0 here
            //combine all compiled units into a single list
            for (int __i = 0; __i < fp_CompiledUnits.size(); __i++)
            {
                fp_FinalByteCode.insert(fp_FinalByteCode.end(), fp_CompiledUnits[__i]->CompiledByteCode.begin(), fp_CompiledUnits[__i]->CompiledByteCode.end());
                pm_CompilationUnitByteOffsets.emplace(fp_CompiledUnits[__i]->TU->ScriptPath.filename().string(), fp_CompiledUnits[__i]->CompiledByteCode.size() + f_CurrentOffset);
                f_CurrentOffset += fp_CompiledUnits[__i]->CompiledByteCode.size();
            }
        }

        for (int __i = 0; __i < fp_CompiledUnits.size(); __i++)
        {
            //resolve symbols if any
            for (const auto& [lv_Key, lv_Val] : fp_CompiledUnits[__i]->TU->UnresolvedSymbols)
            {

            }   
        }

        return BONGO_OK;
    }
}


//BongoScriptUnit f_MainFuncCompilationUnit = move(fp_CompiledUnits.back()); //entrypoint file should be the last element uwu

//now we just walk the include tree and only link code that is actually used oh this should be done by the compiler thread pool uwu