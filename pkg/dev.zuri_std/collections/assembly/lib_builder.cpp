/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_packaging/package_loader.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_command/command_help.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_stream.h>

#include "compile_hashmap.h"
#include "compile_option.h"
#include "compile_treemap.h"
#include "compile_treeset.h"
#include "compile_treeset_iterator.h"
#include "compile_vector.h"
#include "compile_vector_iterator.h"

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
    span->setOperationName("buildZuriZtdCollection");

    auto location = lyric_common::ModuleLocation::fromString("/collections");

    // build the loader chain
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(std::make_shared<lyric_bootstrap::BootstrapLoader>());
    loaderChain.push_back(std::shared_ptr<lyric_packaging::PackageLoader>(
        new lyric_packaging::PackageLoader({ZURI_BUILD_PACKAGES_DIR})));
    auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    auto sharedModuleCache = lyric_importer::ModuleCache::create(loader);

    lyric_assembler::ObjectState objectState(location, sharedModuleCache, &scopeManager);

    // initialize the assembler
    TU_RETURN_IF_NOT_OK (objectState.initialize());

    // define the module entry point
    lyric_compiler::ModuleEntry moduleEntry(&objectState);
    TU_RETURN_IF_NOT_OK (moduleEntry.initialize());

    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *root = moduleEntry.getRoot();
    auto *rootBlock = root->namespaceBlock();

    lyric_assembler::ClassSymbol *TreeSetClass;
    TU_ASSIGN_OR_RETURN (TreeSetClass, declare_std_collections_TreeSet(objectState, rootBlock));
    lyric_assembler::ClassSymbol *TreeSetIteratorClass;
    TU_ASSIGN_OR_RETURN (TreeSetIteratorClass, declare_std_collections_TreeSetIterator(objectState, rootBlock));

    lyric_assembler::ClassSymbol *VectorClass;
    TU_ASSIGN_OR_RETURN (VectorClass, declare_std_collections_Vector(objectState, rootBlock));
    lyric_assembler::ClassSymbol *VectorIteratorClass;
    TU_ASSIGN_OR_RETURN (VectorIteratorClass, declare_std_collections_VectorIterator(objectState, rootBlock));

    TU_RETURN_IF_NOT_OK (build_std_collections_HashMap(objectState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_collections_TreeMap(objectState, rootBlock, typeSystem));

    TU_RETURN_IF_NOT_OK (build_std_collections_TreeSetIterator(
        TreeSetIteratorClass, objectState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_collections_TreeSet(
        TreeSetClass, TreeSetIteratorClass, objectState, rootBlock, typeSystem));

    TU_RETURN_IF_NOT_OK (build_std_collections_VectorIterator(
        VectorIteratorClass, objectState, rootBlock, typeSystem));
    TU_RETURN_IF_NOT_OK (build_std_collections_Vector(
        VectorClass, VectorIteratorClass, objectState, rootBlock, typeSystem));

    TU_RETURN_IF_NOT_OK (build_std_collections_Option(objectState, rootBlock, moduleEntry, typeSystem));

    // serialize state to object
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, objectState.toObject());

    // write object to file
    tempo_utils::FileWriter writer(destinationPath, object.bytesView(),
        tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid())
        return writer.getStatus();

    TU_LOG_INFO << "wrote output to " << destinationPath;
    return {};
}

int
main(int argc, char *argv[])
{
    auto status = build(argc, argv);
    if (!status.isOk())
        tempo_command::display_status_and_exit(status);
    return 0;
}
