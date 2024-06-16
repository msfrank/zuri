/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_packaging/package_loader.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_command/command_help.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_stream.h>

#include "compile_attr.h"
#include "compile_element.h"
#include "compile_future.h"
#include "compile_operation.h"
#include "compile_port.h"
#include "compile_queue.h"
#include "compile_system.h"

tempo_utils::Status
build(int argc, char *argv[])
{
    std::filesystem::path destinationPath;
    if (argc < 2) {
        destinationPath = DEFAULT_DESTINATION_PATH;
    } else {
        destinationPath = argv[1];
    }

    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    auto span = scopeManager.makeSpan();
    span->setOperationName("buildZuriZtdText");

    auto location = lyric_common::AssemblyLocation::fromString("/system");

    // build the loader chain
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(std::make_shared<lyric_bootstrap::BootstrapLoader>());
    loaderChain.push_back(std::shared_ptr<lyric_packaging::PackageLoader>(
        new lyric_packaging::PackageLoader({ZURI_BUILD_PACKAGES_DIR})));
    auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    auto sharedModuleCache = lyric_importer::ModuleCache::create(loader);

    lyric_assembler::AssemblyState assemblyState(location, sharedModuleCache, &scopeManager);

    // initialize the assembler
    TU_RETURN_IF_NOT_OK (assemblyState.initialize());

    // define the module entry point
    lyric_compiler::ModuleEntry moduleEntry(&assemblyState);
    TU_RETURN_IF_NOT_OK (moduleEntry.initialize());

    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *root = moduleEntry.getRoot();
    auto *rootBlock = root->namespaceBlock();

    lyric_assembler::StructSymbol *AttrStruct = nullptr;
    TU_ASSIGN_OR_RETURN (AttrStruct, declare_std_system_Attr(assemblyState, rootBlock));

    lyric_assembler::StructSymbol *ElementStruct = nullptr;
    TU_ASSIGN_OR_RETURN (ElementStruct, declare_std_system_Element(assemblyState, rootBlock));

    TU_RETURN_IF_NOT_OK (build_std_system_Attr(AttrStruct, assemblyState, rootBlock));
    TU_RETURN_IF_NOT_OK (build_std_system_Element(ElementStruct, assemblyState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_system_Future(moduleEntry, rootBlock));

    lyric_assembler::StructSymbol *OperationStruct = nullptr;
    TU_ASSIGN_OR_RETURN (OperationStruct, build_std_system_Operation(assemblyState, rootBlock, AttrStruct, typeSystem));

    TU_RETURN_IF_NOT_OK (build_std_system_AppendOperation(OperationStruct, assemblyState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_system_InsertOperation(OperationStruct, assemblyState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_system_UpdateOperation(OperationStruct, assemblyState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_system_ReplaceOperation(OperationStruct, assemblyState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_system_EmitOperation(OperationStruct, assemblyState, rootBlock, typeSystem));

    TU_RETURN_IF_NOT_OK (build_std_system_Port(OperationStruct, assemblyState, rootBlock));
    TU_RETURN_IF_NOT_OK (build_std_system_Queue(assemblyState, rootBlock));
    TU_RETURN_IF_NOT_OK (build_std_system(assemblyState, rootBlock, typeSystem));

    // serialize state to object
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, assemblyState.toAssembly());

    // write object to file
    tempo_utils::FileWriter writer(destinationPath, object.bytesView(),
        tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        return writer.getStatus();
    }

    TU_LOG_INFO << "wrote output to " << destinationPath;
    return tempo_utils::Status();
}

int
main(int argc, char *argv[])
{
    auto status = build(argc, argv);
    if (!status.isOk())
        tempo_command::display_status_and_exit(status);
    return 0;
}
