import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def summary_backend_io():
    print("================================")
    print("         Backend IOs            ")
    print("================================")
    pending_io = gdb.parse_and_eval("singletonInfo->flushCount->pendingFlush.pendingCount._M_i")
    print("Flushed pending IO : %s" % (str(pending_io)))
    pending_io = gdb.parse_and_eval("singletonInfo->flushCount->callbackNotCalledCount.pendingCount._M_i")
    print("Callback execute not called : %s" % (str(pending_io)))

    pending_io = gdb.parse_and_eval("singletonInfo->ioSubmitHandlerCount->pendingWrite.pendingCount._M_i")
    print("IOSubmitHandler Pending Write : %s" % (str(pending_io)))

    pending_io = gdb.parse_and_eval("singletonInfo->ioSubmitHandlerCount->pendingRead.pendingCount._M_i")
    print("IOSubmitHandler Pending Read : %s" % (str(pending_io)))

    pending_io = gdb.parse_and_eval("singletonInfo->ioSubmitHandlerCount->pendingByteIo.pendingCount._M_i")
    print("IOSubmitHandler Pending ByteIo : %s" % (str(pending_io)))


def summary_io_worker():
    output = gdb.execute("p pos::singletonInfo->ioDispatcher->ioWorkerMap", to_string=True)
    outputlines = output.split('\n')

    print("================================")
    print("     IO Worker Pending IO       ")
    print("================================")
    for output_elem in outputlines:
        if ("]" in output_elem):
            io_worker = output_elem.split(
                    '=')[1].rstrip(' \n').lstrip(' ')
            pending_io_in_ioworker = gdb.parse_and_eval("((IOWorker *) " + io_worker + ")->currentOutstandingIOCount")
            print("IOWorker, %s, pending IO : %s" % (io_worker, str(pending_io_in_ioworker)))


def backend_io():
    summary_io_worker()
    summary_backend_io()
