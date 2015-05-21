
#ifndef __SCHEDULE_RUNNER_H__
#define __SCHEDULE_RUNNER_H__

#include <IContainer.h>
#include <IDispatchable.h>
#include <Shell.h>

class ScheduleRunner: public Shell
{
private:
  bool quit;
  IContainer* remaining_schedule;
public:
  ScheduleRunner(IContainer* _remaining_schedule = NULL): quit(true), remaining_schedule(_remaining_schedule) {}
  void run(const Dispatcher& commands);
  void run(const Dispatcher& commands, const Clock& clock);
};

#endif // __SCHEDULE_RUNNER_H__
