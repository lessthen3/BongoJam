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

    struct BongoLinker //links together compilation units and validates symbols exist and function properly
    {
    private:
        unique_ptr<Logger> linker_logger = nullptr;

    public:
        BongoLinker() 
        {
            linker_logger = make_unique<Logger>();
            linker_logger->Initialize("./logs", "BongoLinker", Logger::LogLevel::ALL_LOGS);

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

    private:
        map<string, CompilationUnit> pm_CompilationUnits; // file name : corresponding compiled code

    };
}