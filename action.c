#include "action.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdbool.h>

static _Bool previous_state = false;
static _Bool keyboard_action(void);
static _Bool pin_action(void);

_Bool action(void)
{
  if (keyboard_action() || pin_action()) {
    if (previous_state) {
      return false; // Trigger only once
    } else {
      previous_state = true;
      return true;
    }
  } else {
    previous_state = false;
    return false;
  }
}

static _Bool keyboard_action(void)
{
  static _Bool stdin_failed = false;
  if (stdin_failed) {
    return false;
  }

  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  if (FD_ISSET(0, &fds)) {
    char buf[1024];
    ssize_t count = read(0, buf, sizeof(buf)); // Empty the buffer (or at least some of it)
    if (count <= 0) {
      // We hit EOF or an error on stdin, disable keyboard
      stdin_failed = true;
    }
    return true;
  } else {
    return false;
  }
}

#ifndef __arm__ // Not on a Raspberry Pi

_Bool pin_action(void)
{
  return false;
}

#else // __arm__

#include <wiringPi.h>

extern int trigger_pin, trigger_level;

_Bool pin_action(void)
{
  static _Bool inited = false;
  if (!inited) {
    wiringPiSetupSys();
  }
  return digitalRead(trigger_pin) == trigger_level;
}

#endif // __arm__
