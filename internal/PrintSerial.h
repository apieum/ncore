
#ifndef __PRINTSERIAL_H__
#define __PRINTSERIAL_H__

#include <IContainer.h>
#include <IDispatchable.h>
#include <Shell.h>

class PrintSerial: public Shell
{
private:
  bool quit;
  IContainer* remaining_schedule;
public:
  PrintSerial(IContainer* _remaining_schedule = NULL): quit(true), remaining_schedule(_remaining_schedule) {}
  void run(const Dispatcher& commands);
  void run(const Dispatcher& commands, const Clock& clock);
};

#endif // __PRINTSERIAL_H__
