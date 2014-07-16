#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#include <arch/bathos-arch.h>

struct statemachine_state;
struct statemachine_runtime;

enum statemachine_type {
	MOORE = 0,
	MEALEY,
};

typedef void (state_outfunc)(const struct statemachine_state * PROGMEM,
			     struct statemachine_runtime *);
typedef state_outfunc *pstate_outfunc;

struct statemachine_state {
	/* There shall be as many next states as are the valid events */
	const int * PROGMEM next_states;
	/* 
	 * This array contains one entry for moore machines, one entry per
	 * valid event for mealey machines
	 */
	state_outfunc * const * PROGMEM out;
};

struct statemachine_runtime {
	int curr_state;
};

static inline int statemachine_get_state(struct statemachine_runtime *r)
{
	return r->curr_state;
}

struct statemachine {
	enum statemachine_type type;
	int nstates;
	const struct statemachine_state * PROGMEM states;
	int nevents;
};

extern int init_statemachine(const struct statemachine * PROGMEM,
			     struct statemachine_runtime *runtime,
			     int initial_state);
extern int feed_statemachine(const struct statemachine * PROGMEM,
			     struct statemachine_runtime *runtime,
			     int event);

#endif /* __STATEMACHINE_H__ */
