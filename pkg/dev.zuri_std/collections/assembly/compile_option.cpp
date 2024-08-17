/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>

#include "compile_option.h"

tempo_utils::Status
build_std_collections_Option(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_typing::TypeSystem *typeSystem)
{
    auto compileOptionResult = moduleEntry.compileClass(R"(
        defclass Option[+T] {

            val _value: T | Nil

            init(value: T | Nil = nil) {
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
                    when v: T       v
                    else            other
                }
            }
        }
        )",
        parentBlock);

    TU_RAISE_IF_STATUS(compileOptionResult);

    return {};
}
