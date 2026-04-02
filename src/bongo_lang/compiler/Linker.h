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

#include "Compiler.h"

namespace BongoJam {

    //////////////////////////////////////////////
    // Import Verification (warn circular dependencies)
    //////////////////////////////////////////////

    struct ImportTree //should parse this tbh idk
    {

        bool
            ValidateImportTree()
        {
            return false;
        }
    };

    struct BongoLinker //links together compilation units and validates symbols exist and function properly
    {
    public:
        BongoLinker() 
        {
            linker_logger = Logger::CreateUnique("BongoLinker", PEACH_LOGGER_DEFAULT_FLAGS, PEACH_LOGGER_DEFAULT_OUTPUT_DIR);

            if (not linker_logger)
            {
                throw runtime_error("WTF MANG LOGGER FAILED TO INITIALIZE FROM BongoLinker WTF MANG");
            }

        }

        ~BongoLinker() = default;

    public:
        int
            LinkCompilationUnits
            (
                vector<BongoScriptUnit>&& fp_CompiledUnits, 
                vector<uint8_t>& fp_FinalByteCode
            );

        bool
            ResolveSymbol();

    private:
        unique_ptr<Logger> linker_logger = nullptr;
        unordered_map<string, uint64_t> pm_CompilationUnitByteOffsets;
    };
}