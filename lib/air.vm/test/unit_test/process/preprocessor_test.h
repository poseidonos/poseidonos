#pragma once

#include "fake_global_meta_getter.h"
#include "fake_node_manager.h"
#include "fake_node_meta_getter.h"
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/Writer.cpp"
#include "src/collection/writer/Writer.h"
#include "src/process/Preprocessor.cpp"
#include "src/process/Preprocessor.h"

class PreprocessorTest : public ::testing::Test
{
public:
    process::Preprocessor* preprocessor{nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter{nullptr};
    FakeGlobalMetaGetter* fake_global_meta_getter{nullptr};
    FakeNodeManager* fake_node_manager{nullptr};

protected:
    PreprocessorTest()
    {
        fake_node_meta_getter = new FakeNodeMetaGetter;
        fake_global_meta_getter = new FakeGlobalMetaGetter;
        fake_node_manager = new FakeNodeManager{fake_global_meta_getter,
            fake_node_meta_getter};
        preprocessor = new process::Preprocessor{fake_node_meta_getter,
            fake_node_manager};
    }
    virtual ~PreprocessorTest()
    {
        if (nullptr != fake_node_meta_getter)
        {
            delete fake_node_meta_getter;
            fake_node_meta_getter = nullptr;
        }
        if (nullptr != fake_global_meta_getter)
        {
            delete fake_global_meta_getter;
            fake_global_meta_getter = nullptr;
        }
        if (nullptr != preprocessor)
            delete preprocessor;
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
