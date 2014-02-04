
#include <bathos/errno.h>
#include <bathos/statemachine.h>


int init_statemachine(const struct statemachine *m, int initial_state)
{
	int i;
	if (m->nstates <= 0 || !m->states || !m->nevents)
		return -EINVAL;
	if (initial_state < 0 || initial_state >= m->nstates)
		return -EINVAL;
	for (i = 0; i < m->nruntimes; i++)
		m->runtimes[i].curr_state = initial_state;
	return 0;
}

int feed_statemachine(const struct statemachine *m, int instance, int event)
{
	const struct statemachine_state *next;
	const struct statemachine_state *c;
	state_outfunc *outf;
	struct statemachine_runtime *runtime;

	if (event < 0 || event >= m->nevents)
		return -EINVAL;
	if (instance < 0 || instance >= m->nruntimes)
		return -EINVAL;
	runtime = &m->runtimes[instance];
	c = &m->states[runtime->curr_state];
	next = &m->states[c->next_states[event]];
	if (!next)
		return -EINVAL;
	runtime->curr_state = next - m->states;
	outf = m->type == MOORE ? next->out[0] : c->out[event];
	if (outf)
		outf(next, runtime);
	return 0;
}
