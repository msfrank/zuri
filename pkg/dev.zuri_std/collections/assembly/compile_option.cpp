/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_option.h"

tempo_utils::Status
build_std_collections_Option(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_typing::TypeSystem *typeSystem)
{
    auto compileOptionResult = moduleEntry.compileClass(R"(
        defclass Option[+T] {

            val _value: T | Nil

            init(named value: T | Nil = nil) {
                set this._value = value
            }

            def IsEmpty(): Bool {
                not (this._value ^? Nil)
            }

            def Get(): T | Nil {
                this._value
            }

            def GetOrElse(other: T): T {
                match this._value {
                    case v: T       v
                    else            other
                }
            }
        }
        )",
        parentBlock);

    TU_RAISE_IF_STATUS(compileOptionResult);

    return {};
}
