// STL includes
#include <iostream>
// C includes
// Library includes
// Project includes
#include <Dispatcher.h>
#include <Clock.h>
#include <ScheduleRunner.h>

/****************************************************************************/

using namespace std;

/****************************************************************************/

void ScheduleRunner::run(const Dispatcher& _commands)
{
  run(_commands, Clock());
}

/****************************************************************************/

void ScheduleRunner::run(const Dispatcher& _commands, const Clock& _clock)
{
  while ( remaining_schedule && remaining_schedule->size() )
  {
  }
  _commands.execute("quit");
  _clock.delay(1);
}

/****************************************************************************/

// vim:cin:ai:sts=2 sw=2 ft=cpp