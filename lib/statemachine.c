
#include <bathos/errno.h>
#include <bathos/statemachine.h>

#if defined ARCH_IS_HARVARD

static struct statemachine *
__get_statemachine(const struct statemachine * PROGMEM _m,
		   struct statemachine *m)
{
	memcpy_p(m, _m, sizeof(*m));
	return m;
}

static struct statemachine_state *
__get_state(struct statemachine *m, int sn, struct statemachine_state *s)
{
	memcpy_p(s, &m->states[sn], sizeof(*s));
	return s;
}

static int __get_next(struct statemachine_state *s, int e)
{
	int out;
	__copy_int(&out, &s->next_states[e]);
	return out;
}

static pstate_outfunc __get_outf(struct statemachine_state *s, int e)
{
	pstate_outfunc out;
	memcpy_p(&out, &s->out[e], sizeof(out));
	return out;
}

#else

static struct statemachine *
__get_statemachine(const struct statemachine * PROGMEM _m,
		   struct statemachine *m)
{
	return _m;
}

static struct statemachine_state *
__get_state(struct statemachine *m, int sn, struct statemachine_state *s)
{
	return &m->states[sn];
}

static int __get_next(struct statemachine_state *s, int e)
{
	return s->next_states[e];
}

static pstate_outfunc __get_outf(struct statemachine_state *s, int e)
{
	return &s->out[e];
}

#endif


int init_statemachine(const struct statemachine * PROGMEM _m,
		      struct statemachine_runtime *rt,
		      int initial_state)
{
	struct statemachine *m, __m;

	m = __get_statemachine(_m, &__m);
	if (m->nstates <= 0 || !m->states || !m->nevents)
		return -EINVAL;
	if (initial_state < 0 || initial_state >= m->nstates)
		return -EINVAL;
	rt->curr_state = initial_state;
	return 0;
}

int feed_statemachine(const struct statemachine * PROGMEM _m,
		      struct statemachine_runtime *rt,
		      int event)
{
	struct statemachine *m, __m;
	struct statemachine_state *next, __next;
	struct statemachine_state *c, __c;
	pstate_outfunc outf;

	m = __get_statemachine(_m, &__m);
	if (event < 0 || event >= m->nevents)
		return -EINVAL;
	if (!rt)
		return -EINVAL;
	c = __get_state(m, rt->curr_state, &__c);
	next = __get_state(m, __get_next(c, event), &__next);
	if (!next)
		return -EINVAL;
	rt->curr_state = __get_next(c, event);
	outf = m->type == MOORE ? __get_outf(next, 0) : __get_outf(c, event);
	if (outf)
		outf(next, rt);
	return 0;
}
