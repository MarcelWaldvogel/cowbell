#include "flexosip/flexosip.h"
#include "flexosip/flexosnd.h"
#include "flexosip/flexortp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ini.h>
#include "flexosip/unused.h"
#include <signal.h>
#include "action.h"

#define CONFIG1 "/etc/cowbell.ini"
#define CONFIG2 "cowbell.ini"

// Registration parameters
static char *uri, *registrar, *login, *password;
// Call parameters
static char *destination, *name;
static int ring_duration = -1;
// Trigger parameters
int trigger_pin = -1, trigger_level = -1;
// Media
static char *welcome = "media/hello.ogg";

static int handle_ini(void* UNUSED_PARAM(user), const char* section,
		      const char* name, const char* value)
{
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH("registration", "uri")) {
    uri = strdup(value);
  } else if (MATCH("registration", "registrar")) {
    registrar = strdup(value);
  } else if (MATCH("registration", "login")) {
    login = strdup(value);
  } else if (MATCH("registration", "password")) {
    password = strdup(value);
  } else if (MATCH("alert", "destination")) {
    destination = strdup(value);
  } else if (MATCH("alert", "name")) {
    name = strdup(value);
  } else if (MATCH("alert", "duration")) {
    ring_duration = atoi(value);
  } else if (MATCH("trigger", "gpio")) {
    trigger_pin = atoi(value);
  } else if (MATCH("trigger", "level")) {
    trigger_level = atoi(value);
  } else if (MATCH("media", "welcome")) {
    welcome = strdup(value);
  } else {
    fprintf(stderr, "Unknown config option [%s] %s=%s\n", section, name, value);
    return 0;  /* unknown section/name, error */
  }
  return 1;
}

static _Bool watchful = false, incoming_call = false, outgoing_call = false;
time_t until = 0, hangup_at = 0;

void play_watchful_status(void)
{
  if (watchful) {
    if (until == 0) {
      fesip_play_after_delay(200, "media/1x.ogg");
    } else if (until == -1) {
      fesip_play_after_delay(200, "media/perm.ogg");
    } else {
      fesip_play_after_delay(200, "media/5min.ogg");
    }
  } else {
    fesip_play_after_delay(200, "media/off.ogg");
  }
}

int main(int UNUSED_PARAM(argc), char **UNUSED_PARAM(argv))
{
#ifdef ENABLE_TRACE
    TRACE_INITIALIZE(OSIP_INFO1, NULL);
#endif

  if (ini_parse(CONFIG1, handle_ini, NULL) < 0 &&
      ini_parse(CONFIG2, handle_ini, NULL)) {
    fprintf(stderr, "Cannot load either " CONFIG1 " or " CONFIG2 "\n");
    return 1;
  }
  if (uri == NULL || registrar == NULL || login == NULL || password == NULL ||
      destination == NULL) {
    fprintf(stderr, "Missing configuration value\n");
    return 1;
  }
  if (trigger_pin < 0 || trigger_level < 0) {
    fprintf(stderr, "Missing trigger configuration values\n");
    return 1;
  }
  signal(SIGINT, fesip_cleanup);
  signal(SIGHUP, fesip_cleanup);
  signal(SIGTERM, fesip_cleanup);
  signal(SIGQUIT, fesip_cleanup);
  fesip_listen(IPPROTO_UDP, false, 0);
  fesip_register(uri, registrar, login, password);
  int i = fesip_wait_registered();
  if (i < 0) {
    fprintf(stderr, "Registration failed: %d\n", i);
    exit(1);
  } else {
    fprintf(stderr, "Registration succeeded, continuing...\n");
  }
  while (1) {
    // Handle SIP and RTP (audio)
    fesip_handle_event();

    // Should we alert?
    if (watchful) {
      if (until != 0 && until != -1) {
	time_t now = time(NULL);
	if (now > until) {
	  // Disable alarm
	  watchful = false;
	}
      }
      if (action()) {
	if (until == 0) {
	  watchful = false;
	}
	outgoing_call = true;
	if (ring_duration > 0) {
	  time_t now = time(NULL);
	  hangup_at = now + ring_duration;
	}
	fesip_call(uri, destination, name, NULL);
      }
    }

    // Anyone calling in?
    if (incoming_call) {
      incoming_call = false;
      fesip_answer();
      fesip_play_after_delay(500, welcome);
      if (!watchful) {
	// 1x on
	watchful = true;
	until = 0;
	fesip_play_after_delay(200, "media/on.ogg");
        // Single activation: Ensure that any pending trigger is consumed first.
	// Otherwise, a callback will be initiated right away, which will fail
	// due to the single-call limit, but will reset `watchful`.
        action();
      } else {
	play_watchful_status();
      }
    }

    // Ring duration
    if (outgoing_call) {
      time_t now = time(NULL);
      if (hangup_at > 0 && now >= hangup_at) {
	fesip_terminate();
      }
    }
  }
}

void fesip_event_answered(eXosip_event_t *UNUSED_PARAM(evt),
    const char *UNUSED_PARAM(host), int UNUSED_PARAM(port), int UNUSED_PARAM(format))
{
  fesip_terminate_nolock(); // The lock is already acquired
}

void fesip_event_terminate(eXosip_event_t *UNUSED_PARAM(evt))
{
  incoming_call = outgoing_call = false;
  fprintf(stderr, "Call terminated\n");
}

int fesip_event_invite(eXosip_event_t *UNUSED_PARAM(evt),
    const char *UNUSED_PARAM(host), int UNUSED_PARAM(port), int UNUSED_PARAM(format))
{
  fprintf(stderr, "Ringing for incoming call\n");
  incoming_call = true;
  return SIP_RINGING;
}

void fesip_event_dtmf(char c)
{
  fesnd_close(); // Terminate all ongoing messages
  switch (c)
  {
  case '0':
    watchful = false;
    fesip_play_after_delay(300, "media/off.ogg");
    break;
  case '1':
    watchful = true;
    until = 0;
    fesip_play_after_delay(300, "media/1x.ogg");
    // Single activation: Ensure that any pending trigger is consumed first
    action();
    break;
  case '5':
    watchful = true;
    time_t now = time(NULL);
    until = now + 300;
    fesip_play_after_delay(300, "media/5min.ogg");
    break;
  case '9':
    watchful = true;
    until = -1;
    fesip_play_after_delay(300, "media/perm.ogg");
    break;
  default:
    fprintf(stderr, "Received unrecognized DTMF code %c\n", c);
    break;
  }
}
