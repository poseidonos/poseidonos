import unittest
import filter_out_wrong_branches as sut


class TestFilteringOutRegex(unittest.TestCase):
    def test_empty_file(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        ''')
        self.assertEqual([], actual)

    def test_file_containing_no_branch(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        #pragma once

        #include "src/array/array.h"
        #include "src/array_components/meta_mount_sequence.h"
        #include "src/gc/garbage_collector.h"
        #include "src/journal_manager/journal_manager.h"
        #include "src/mapper/mapper.h"
        #include "src/volume/volume_manager.h"
        #include "src/metafs/metafs.h"
        #include "src/io/general_io/rba_state_manager.h"
        #include <vector>
        #include <string>
        #include <functional>

        using namespace std;
        ''')
        self.assertEqual([], actual)

    def test_containing_single_if(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        ArrayComponents::~ArrayComponents(void)
        {
            POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Deleting array component for {}", arrayName);

            _DestructMetaComponentsInOrder();
            if (arrayMountSequence != nullptr)
            {
                delete arrayMountSequence;
                arrayMountSequence = nullptr;
                POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "ArrayMountSequence for {} has been deleted.", arrayName);
            }
        ''')
        self.assertEqual([(6, 7)], actual)

    def test_containing_multiple_if(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        void
        ArrayComponents::_DestructMetaComponentsInOrder(void)
        {
            // Please note that the order of creation should be like the following:
            if (metaMountSequence != nullptr)
            {
                delete metaMountSequence;
                metaMountSequence = nullptr;
                POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "MetaMountSequence for {} has been deleted.", arrayName);
            }

            if (gc != nullptr)
            {
                delete gc;
                gc = nullptr;
                POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "GarbageCollector for {} has been deleted.", arrayName);
            }
        ''')
        self.assertEqual([(5, 6), (12, 13)], actual)

    def test_containing_multiline_if(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        void
        ArrayComponents::_InstantiateMetaComponentsAndMountSequenceInOrder(bool isArrayLoaded)
        {
            if (metafs != nullptr
                || volMgr != nullptr
                || mapper != nullptr
                || allocator != nullptr
                || journal != nullptr
                || gc != nullptr
                || metaMountSequence != nullptr)
            {
                POS_TRACE_WARN(EID(ARRAY_COMPONENTS_LEAK), "Meta Components exist already. Possible memory leak (or is it a mock?). Skipping.");
                return;
            }
        ''')
        self.assertEqual([(4, 11)], actual)

    def test_containing_multiple_if_else(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        if (config->IsEnabled() == true)
        {
            result = _InitConfigAndPrepareLogBuffer(metaFsCtrl);
            if (result < 0)
            {
                return result;
            }

            _InitModules(vsaMap, stripeMap, mapFlush, blockAllocator,
                wbStripeAllocator, ctxManager, ctxReplayer, volumeManager, eventScheduler);

            if (journalManagerStatus == WAITING_TO_BE_REPLAYED)
            {
                result = _DoRecovery();
            }
            else
            {
                result = _Reset();
            }
        }
        ''')
        self.assertEqual([(1, 2), (4, 5), (12, 13), (16, 17)], actual)

    def test_containing_nested_while_and_if_else(self):
        actual = sut.get_if_else_while_expr_sorted_intervals('''
        while (splitVolumeIoQueue.empty() == false)
        {
            VolumeIoSmartPtr volumeIo = splitVolumeIoQueue.front();
            bool skipIoSubmission = (false == volumeIo->CheckPbaSet());
            if (skipIoSubmission)
            {
                IoCompleter ioCompleter(volumeIo);
                ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
            }
            else
            {
                _SendVolumeIo(volumeIo);
            }
            splitVolumeIoQueue.pop();
        }
        ''')
        self.assertEqual([(1, 2), (5, 6), (10, 11)], actual)


class TestShouldFilterOutRule(unittest.TestCase):

    mock_src_line = "POS_DEBUG(SUCCESS, 'blahblah');"

    def test_src_line_before_the_very_first_branch(self):
        intervals = [(10, 15), (20, 30)]
        actual = sut.should_filter_out(self.mock_src_line, 5, intervals)
        self.assertTrue(actual)

    def test_src_line_in_the_middle_of_a_branch(self):
        intervals = [(10, 15), (20, 30)]
        actual = sut.should_filter_out(self.mock_src_line, 13, intervals)
        self.assertFalse(actual)

        actual = sut.should_filter_out(self.mock_src_line, 17, intervals)
        self.assertTrue(actual)

        actual = sut.should_filter_out(self.mock_src_line, 25, intervals)
        self.assertFalse(actual)

    def test_src_line_after_the_last_branch(self):
        intervals = [(10, 15), (20, 30)]
        actual = sut.should_filter_out(self.mock_src_line, 35, intervals)
        self.assertTrue(actual)


if __name__ == '__main__':
    unittest.main()
