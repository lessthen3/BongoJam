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
#include "../../../include/bongo_lang/compiler/Linker.h"

namespace BongoJam{

    bool
        BongoLinker::WriteBytecodeToFile
        (
            const vector<uint8_t>& fp_ByteCode,
            const string& fp_DesiredOutputDirectory,
            const string& fp_DesiredName
        )
    {
        // Ensure directory exists
        if (not filesystem::exists(fp_DesiredOutputDirectory))
        {
            linker_logger->Error(format("Tried to pass invalid write directory: '{}' to WriteBytecodeToFile()", fp_DesiredOutputDirectory), "BongoLinker");
            return false;
        }
        else if (fp_ByteCode.empty())
        {
            linker_logger->Error("Failed to write " + fp_DesiredName + " for writing. No bytecode found to write.", "BongoLinker");
            return false;
        }

        linker_logger->Info(format("Bytecode size: {}", fp_ByteCode.size()), "BongoLinker");

        string f_BongoFileName;

        if (fp_DesiredOutputDirectory == "./")// ????????
        {
            f_BongoFileName = "./" + fp_DesiredName + ".bongo";
        }
        else
        {
            f_BongoFileName = fp_DesiredOutputDirectory + "/" + fp_DesiredName + ".bongo";
        }

        ofstream file(f_BongoFileName, ios::binary);  // Open in binary mode

        if (not file)
        {
            linker_logger->Error("Compiler Error: Failed to open " + f_BongoFileName + " for writing.", "BongoLinker");
            return false;
        }

        // Write the entire contents of the vector to the file
        file.write(reinterpret_cast<const char*>(fp_ByteCode.data()), fp_ByteCode.size());

        file.close();  // Close the file

        return true;
    }

    int
        BongoLinker::LinkCompilationUnits(vector<BongoScriptUnit >&& fp_CompiledUnits, vector<uint8_t>& fp_FinalByteCode)
    {

        return BONGO_OK;
    }
}