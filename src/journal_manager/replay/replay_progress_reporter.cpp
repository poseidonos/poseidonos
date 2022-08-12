/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "replay_progress_reporter.h"

#include <unistd.h>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ReplayProgressReporter::ReplayProgressReporter(void)
: totalWeight(0),
  progress(0),
  currentTaskProgress(0),
  reportedProgress(-1)
{
}

void
ReplayProgressReporter::RegisterTask(ReplayTaskId taskId, int taskWeight)
{
    TaskProgress progress(taskWeight);
    taskProgressList.emplace(taskId, progress);

    totalWeight += taskWeight;
}

void
ReplayProgressReporter::TaskStarted(ReplayTaskId taskId, int numTasks)
{
    currentTaskProgress = 0;
    taskProgressList[taskId].Start(numTasks);
    _ReportProgress();
}

void
ReplayProgressReporter::SubTaskCompleted(ReplayTaskId taskId, int completedTasks)
{
    taskProgressList[taskId].SubTaskCompleted(completedTasks);
    currentTaskProgress = taskProgressList[taskId].GetCurerntProgress();
    _ReportProgress();
}

void
ReplayProgressReporter::TaskCompleted(ReplayTaskId taskId)
{
    taskProgressList[taskId].Complete();
    progress += taskProgressList[taskId].GetCurerntProgress();
    currentTaskProgress = 0;
    _ReportProgress();
}

void
ReplayProgressReporter::CompleteAll(void)
{
    progress = totalWeight;
    currentTaskProgress = 0;
    _ReportProgress();
}

int
ReplayProgressReporter::GetProgress(void)
{
    return currentTaskProgress;
}

int
ReplayProgressReporter::GetReportedProgress(void)
{
    return reportedProgress;
}

int
ReplayProgressReporter::GetTotalWeight(void)
{
    return totalWeight;
}

const TaskProgress
ReplayProgressReporter::GetTaskProgress(ReplayTaskId taskId)
{
    return taskProgressList[taskId];
}

void
ReplayProgressReporter::_ReportProgress(void)
{
    if (totalWeight == 0)
    {
        return;
    }

    int percent = (progress + currentTaskProgress) * 100 / totalWeight;

    if (percent != reportedProgress)
    {
        POS_REPORT_TRACE(EID(SYSTEM_RECOVERY),
            "progress report: [" + to_string(percent) + "]");
        usleep(100 * 1000); // delay for the m-tool integration
        reportedProgress = percent;
    }
}

} // namespace pos
