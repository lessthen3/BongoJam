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
            linker_logger = make_unique<Logger>();
            linker_logger->Initialize(DEFAULT_LOG_OUTPUT_DIRECTORY, "BongoLinker", DEFAULT_LOG_LEVEL_FILTER);

        }

        ~BongoLinker() = default;

    public:
        bool
            WriteBytecodeToFile
            (
                const vector<uint8_t>& fp_ByteCode,
                const string& fp_DesiredOutputDirectory,
                const string& fp_DesiredName
            );

        int
            LinkCompilationUnits(vector<BongoScriptUnit>&& fp_CompiledUnits, vector<uint8_t>& fp_FinalByteCode);

    private:
        unique_ptr<Logger> linker_logger = nullptr;
    };
}