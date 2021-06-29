#  Event Handler Implementation Guide

## 1. Basic Implementation
If you want to add an event handler which will be serviced on an event worker on
a particular cpu core, you need to inherit from the `Event` class for your
specific event handler and implement the needed routine in `Execute` method.
***

## 2. Constraint
The `Execute` method SHOULD NOT HAVE ANY ROUTINES waiting another event handler
to be completed by enqueuing another event from each thread context which means,
*
DO NOT IMPLEMENT AS A SYNC OPERATION THAT ENQUEUES AND WAITS ANOTHER EVENT TO BE
COMPLETED.
*

If this happens on a particular event handler on an event worker,
that particular additional event can be enqueued on the very current
event worker's event queue which will be eventually stuck by **DEADLOCK**.
(The event I'm waiting is behind me!!!!!)

So, DO NOT WAIT if you need to go forward with additional event handler.
Instead, make your own state machine and return false then the event worker
will reschedule your event enquing into event scheduler, or split them into
two or more events so that each can be serialized without blocking each event
worker.

Please refer to the wiki page for more detailed information about this,
  <http://globalwiki.itplatform.sec.samsung.net:8099/pages/viewpage.action?pageId=8508275>