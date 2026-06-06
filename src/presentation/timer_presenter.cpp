#include <presentation/timer_presenter.h>

#include <app/app_state.h>
#include <ui/timer.h>

namespace {
const char *timerStatusText() {
  if (isTimerRunning())
    return "RUNNING";
  if (isTimerPaused())
    return "PAUSED";
  return "STOP";
}

TimerView buildTimerView(int secondsLeft) {
  TimerView view;
  view.title = "Timer";
  view.status = timerStatusText();
  view.secondsLeft = secondsLeft;
  view.totalSeconds = app.timer.timerTotalSec;
  view.showProgress = !isTimerStopped();
  return view;
}
} // namespace

void timerRender(int secondsLeft) {
  drawTimerScreen(buildTimerView(secondsLeft));
}
