/**************************************************************************
 *                         BongoJam Script v0.0.1
 *                  Created by Ranyodh Mandur - 🗻2026
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
#include <string>

#include <cstdint>

namespace BongoJam{

    //////////////////////////////////////////////////////////////////////////
    // SSA_OP — the opcode set for SSA IR.
    //
    // NOT the same as BJ_OP. Key differences:
    //   - No PUSH/POP/ENTER/LEAVE (no stack concept in SSA)
    //   - Has PHI (doesn't exist in bytecode)
    //   - Has CONST_* (bytecode uses PUSH_* inline)
    //   - JUMP/BRANCH target BasicBlock IDs not byte offsets
    //   - CALL is an SSA instruction (destination = return value)
    //
    // Value ops (ADD_I, CMP_EQ_I, etc.) are intentionally the same
    // names as BJ_OP since they map 1:1 during bytecode lowering.
    //////////////////////////////////////////////////////////////////////////

    enum class SSA_OP : uint16_t
    {
        NOP = 0,

        //------------------------------------------------------------------
        // SSA-specific — no equivalent in BJ_OP
        //------------------------------------------------------------------

        PHI,        // dest = φ(val_from_pred₀, val_from_pred₁, ...)
                    // ExtIdx → PhiTable entry listing (SSAValue, BlockId) pairs

        CONST_I,    // dest = ImmInt   (i64 literal)
        CONST_U,    // dest = ImmUint  (u64 literal)
        CONST_F,    // dest = ImmFloat (f64 literal)
        CONST_S,    // dest = string literal, ImmStrIdx → string pool

        ALLOCA,     // dest = pointer to stack-allocated slot of given type
                    // (for local variables that need addressability)

        //------------------------------------------------------------------
        // Control flow — terminators, always last in a BasicBlock
        //------------------------------------------------------------------

        JUMP,       // unconditional — Src0 = target block ID (as SSAValue, abusing it slightly)
        BRANCH,     // conditional   — Src0 = cond, Src1 = then-block ID, ImmInt = else-block ID
        RETURN,     // Src0 = return value (INVALID_VALUE for void return)

        //------------------------------------------------------------------
        // Arithmetic — maps 1:1 to BJ_OP during bytecode lowering
        //------------------------------------------------------------------

        ADD_I, ADD_U, ADD_F,
        ADD_I_U, ADD_I_F, ADD_F_U,

        SUB_I, SUB_U, SUB_F,
        SUB_I_U, SUB_U_I, SUB_I_F, SUB_F_I, SUB_U_F, SUB_F_U,

        MUL_I, MUL_U, MUL_F,
        MUL_I_F, MUL_I_U, MUL_F_U,

        DIV_I, DIV_U, DIV_F,
        DIV_I_F, DIV_F_I, DIV_I_U, DIV_U_I, DIV_U_F, DIV_F_U,

        MOD_I, MOD_U, MOD_I_U, MOD_U_I,

        NEG_I, NEG_F,   // unary negate

        INC_I, INC_U,
        DEC_I, DEC_U,

        SQRT_F, SQRT_I, SQRT_U,
        POW_I, POW_U, POW_F, POW_I_U, POW_U_I,

        CEIL, FLOOR,

        //------------------------------------------------------------------
        // Bitwise
        //------------------------------------------------------------------

        AND, OR, XOR, NOT,
        SHL, SHR, SAR,

        //------------------------------------------------------------------
        // Comparisons — result is always BJ_Type::Bool
        //------------------------------------------------------------------

        CMP_EQ_I, CMP_EQ_U, CMP_EQ_F,
        CMP_NE_I, CMP_NE_U, CMP_NE_F,
        CMP_LT_I, CMP_LT_U, CMP_LT_F,
        CMP_GT_I, CMP_GT_U, CMP_GT_F,
        CMP_LE_I, CMP_LE_U, CMP_LE_F,
        CMP_GE_I, CMP_GE_U, CMP_GE_F,

        // cross-type comparisons (same as BJ_OP)
        CMP_LT_I_F, CMP_LT_I_U, CMP_LT_U_F,
        CMP_GT_I_F, CMP_GT_I_U, CMP_GT_U_F,
        CMP_LE_I_F, CMP_LE_I_U, CMP_LE_U_F,
        CMP_GE_I_F, CMP_GE_I_U, CMP_GE_U_F,

        LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,

        //------------------------------------------------------------------
        // Type casts — maps 1:1 to BJ_OP
        //------------------------------------------------------------------

        CAST_I_TO_F, CAST_U_TO_F,
        CAST_F_TO_I, CAST_F_TO_U,
        CAST_I_TO_U, CAST_U_TO_I,

        //------------------------------------------------------------------
        // Memory — heap loads/stores via ResourceHandle
        //------------------------------------------------------------------

        LOAD,       // dest = *ptr  (Src0 = SSAValue of ResourceHandle)
        STORE,      // *ptr = val   (Src0 = ptr, Src1 = value, dest = INVALID)
        LOAD_FIELD, // dest = obj.field  (Src0 = obj handle, ImmInt = field index)
        STORE_FIELD,// obj.field = val   (Src0 = obj, Src1 = val, ImmInt = field index)

        //------------------------------------------------------------------
        // Calls
        //------------------------------------------------------------------

        CALL,       // dest = fn(args...)
                    // ImmStrIdx = function name hash (resolved to addr by linker)
                    // ExtIdx → CallArgTable entry listing arg SSAValues

        CALL_NATIVE,// dest = native_fn(args...) — baked into bongo_core
        CALL_EXTERN,// dest = extern_fn(args...) — dynamically loaded

        //------------------------------------------------------------------
        // Strings
        //------------------------------------------------------------------

        ADD_S,                              // string concat
        ADD_S_I, ADD_S_U, ADD_S_F,          // string + primitive
        ADD_I_S, ADD_U_S, ADD_F_S,          // primitive + string

        //------------------------------------------------------------------
        // Intrinsics / syscalls — preserved from BJ_OP
        //------------------------------------------------------------------

        STDOUT, STDERR, STDIN,
        CLOCK_START, CLOCK_END,
        SLEEP, THREAD,

        //------------------------------------------------------------------
        // Debug / reflection
        //------------------------------------------------------------------

        LINE_NUMBER,    // metadata — ImmInt = source line, no dest
        DEBUG_LINE,
        BREAKPOINT,
        TYPE_TAG,       // dest = runtime type tag of Src0
        VARIABLE_ID,

        //------------------------------------------------------------------
        // Error handling
        //------------------------------------------------------------------

        THROW,
        TRY_BEGIN,  // marks start of try region — ImmInt = catch block ID
        TRY_END,
        CATCH,
        EXCEPTION,
    };

        //////////////////////////////////////////////////////////////////////////
     // SSAValue — the "name" of a value in SSA form.
     // Just a monotonically increasing integer ID.
     // x₀, x₁, x₂ in the math → SSAValue 0, 1, 2 in code.
     // INVALID_VALUE means "no value" (void result, unused operand).
     //////////////////////////////////////////////////////////////////////////

    using SSAValue = uint32_t;
    constexpr SSAValue INVALID_VALUE = UINT32_MAX;

    //////////////////////////////////////////////////////////////////////////
    // BJ_Type — the type of an SSA value.
    // Kept simple: BongoJam is statically typed so every SSAValue has
    // exactly one type, known at IR construction time.
    //////////////////////////////////////////////////////////////////////////

    enum class BJ_Type : uint8_t
    {
        Void = 0,
        Int,        // i64
        Uint,       // u64
        Float,      // f64
        Bool,       // i1 effectively, stored as i64 in registers
        String,     // heap pointer to bongo string
        Ptr,        // heap pointer (ResourceHandle index)
        Invalid
    };

    //////////////////////////////////////////////////////////////////////////
     // PhiArg — one incoming edge of a phi node.
     // "if we came from block BlockId, the value was ValueId"
     //////////////////////////////////////////////////////////////////////////

    struct PhiArg
    {
        SSAValue    m_Value;    // the incoming SSAValue
        uint32_t    m_BlockId;  // which predecessor block it came from
    };

    //////////////////////////////////////////////////////////////////////////
  // SSAInstruction — one node in the def-use graph.
  //
  // Layout: every instruction defines at most ONE value (Dest) and
  // consumes at most TWO values inline (Src0, Src1).
  //
  // This covers ~95% of instructions: binary arithmetic, comparisons,
  // unary ops, loads, stores, casts, constants.
  //
  // Variable-arity instructions (PHI, CALL) use ExtIdx to index into
  // the side tables on SSAFunction. No heap allocation per-instruction.
  //////////////////////////////////////////////////////////////////////////

    struct SSAInstruction
    {
        SSA_OP      Op = SSA_OP::NOP;
        BJ_Type     Type = BJ_Type::Invalid;     // type of Dest
        SSAValue    Dest = INVALID_VALUE;         // result (INVALID if void/terminator)
        SSAValue    Src0 = INVALID_VALUE;         // first operand
        SSAValue    Src1 = INVALID_VALUE;         // second operand (INVALID if unary)

        // Index into SSAFunction's side tables for PHI args or call args.
        // INVALID_VALUE means "not used, no side table entry."
        uint32_t    ExtIdx = INVALID_VALUE;

        uint32_t    LineNumber = 0;                  // source location for error messages

        // Immediate value — only meaningful for CONST_* instructions
        union
        {
            int64_t     ImmInt = 0;
            uint64_t    ImmUint;
            double      ImmFloat;
            uint32_t    ImmStrIdx;   // index into SSAFunction::StringPool
        };
    };

    
    //////////////////////////////////////////////////////////////////////////
    // BasicBlock — a straight-line sequence of SSAInstructions with
    // exactly ONE entry point (the top) and ONE exit (the terminator).
    //
    // The terminator is always the last instruction and is one of:
    //   JUMP, BRANCH, RETURN
    //
    // Preds and Succs are block IDs, maintained for CFG traversal
    // (needed for dominance analysis, phi insertion, etc.)
    //////////////////////////////////////////////////////////////////////////

    struct BasicBlock
    {
        uint32_t                    m_Id;
        string                      m_Label;    // optional debug name e.g. "loop.header"

        vector<SSAInstruction>      m_Insts;    // last is always a terminator

        vector<uint32_t>            m_Preds;    // IDs of predecessor blocks
        vector<uint32_t>            m_Succs;    // IDs of successor blocks
    };

    //////////////////////////////////////////////////////////////////////////
    // SSAFunction — one compiled BongoJam function in SSA form.
    //
    // This is what the compiler builds from a FuncDeclaration AST node.
    // Optimization passes operate on this. Lowering reads from this.
    //
    // Side tables (PhiTable, CallArgTable) avoid per-instruction heap alloc
    // while still supporting variable-arity ops.
    //////////////////////////////////////////////////////////////////////////

    struct SSAFunction
    {
        string                          m_Name;
        BJ_Type                         m_ReturnType = BJ_Type::Void;
        vector<pair<string, BJ_Type>>   m_Params;

        vector<BasicBlock>              m_Blocks;   // m_Blocks[0] is the entry block

        // Side tables — indexed by SSAInstruction::ExtIdx
        vector<vector<PhiArg>>          m_PhiTable;         // for PHI instructions
        vector<vector<SSAValue>>        m_CallArgTable;      // for CALL instructions

        // String literal pool — CONST_S instructions index here
        vector<string>                  m_StringPool;

        // Counter for fresh SSAValue IDs — call this every time you need a new name
        uint32_t                        m_NextValueId = 0;

        SSAValue FreshValue() { return m_NextValueId++; }

        // Convenience: allocate a new BasicBlock and return its ID
        inline uint32_t
            NewBlock(const string& fp_Label = "")
        {
            uint32_t f_Id = static_cast<uint32_t>(m_Blocks.size());
            m_Blocks.push_back({ f_Id, fp_Label });
            return f_Id;
        }

        // Convenience: add a phi arg entry to the side table, return ExtIdx
        inline uint32_t
            AddPhiArgs(vector<PhiArg>&& fp_Args)
        {
            uint32_t f_Idx = static_cast<uint32_t>(m_PhiTable.size());
            m_PhiTable.push_back(move(fp_Args));
            return f_Idx;
        }

        // Convenience: add a call arg list to the side table, return ExtIdx
        inline uint32_t
            AddCallArgs(vector<SSAValue>&& fp_Args)
        {
            uint32_t f_Idx = static_cast<uint32_t>(m_CallArgTable.size());
            m_CallArgTable.push_back(move(fp_Args));
            return f_Idx;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // SSAModule — the full SSA IR for one CompilationUnit.
    // This replaces vector<SSAInstruction> in CompilationUnit.
    //////////////////////////////////////////////////////////////////////////

    struct SSAModule
    {
        string                  m_ScriptPath;
        vector<SSAFunction>     m_Functions;
    };


//////////////////////////////////////////////////////////////////////////////
// How CompilationUnit changes:
//
//   struct CompilationUnit
//   {
//       unique_ptr<TranslationUnit> TU;
//       SSAModule                   SSA;          // ← replaces vector<SSAInstruction>
//       vector<uint8_t>             CompiledByteCode; // ← lowered FROM SSA
//   };
//
// The lowering path:
//   AST (TranslationUnit) → SSA (SSAModule) → bytecode (CompiledByteCode)
//   AST (TranslationUnit) → SSA (SSAModule) → AsmJit emission → .bongoc blob
//
// Both consumers (bytecode serializer and AsmJit emitter) read SSAModule.
// Optimization passes (DCE, folding, etc.) mutate SSAModule in place.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// QUICK EXAMPLE: compiling  var x -> int = 0; var y -> int = x + 1;
//
// Compiler calls FreshValue() to get IDs:
//   SSAValue x0 = fn.FreshValue();  // = 0
//   SSAValue y0 = fn.FreshValue();  // = 1
//   SSAValue one = fn.FreshValue(); // = 2
//   SSAValue add = fn.FreshValue(); // = 3
//
// Emits to entry block:
//   { CONST_I, Int, x0,  INVALID, INVALID, INVALID, 1, {.ImmInt=0}  }
//   { CONST_I, Int, one, INVALID, INVALID, INVALID, 2, {.ImmInt=1}  }
//   { ADD_I,   Int, add, x0,      one,     INVALID, 3, {}           }
//   (y0 IS add — same SSAValue, no copy needed)
//
// For an if branch — see the phi example above in the diagram.
}