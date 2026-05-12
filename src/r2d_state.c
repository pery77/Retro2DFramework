#include "r2d/r2d.h"

static void R2D_StateEnter(R2D_StateMachine *machine, R2D_State *state)
{
    if (state->enter != 0) {
        state->enter(state->data, machine->user_data);
    }
}

static void R2D_StateExit(R2D_StateMachine *machine, R2D_State *state)
{
    if (state->exit != 0) {
        state->exit(state->data, machine->user_data);
    }
}

void R2D_StateMachineInit(R2D_StateMachine *machine, void *user_data)
{
    if (machine == 0) {
        return;
    }

    *machine = (R2D_StateMachine) { 0 };
    machine->user_data = user_data;
}

void R2D_StateMachineClear(R2D_StateMachine *machine)
{
    if (machine == 0) {
        return;
    }

    while (machine->count > 0) {
        R2D_StateMachinePop(machine);
    }
}

bool R2D_StateMachineSet(R2D_StateMachine *machine, R2D_State state)
{
    if (machine == 0) {
        return false;
    }

    R2D_StateMachineClear(machine);
    return R2D_StateMachinePush(machine, state);
}

bool R2D_StateMachinePush(R2D_StateMachine *machine, R2D_State state)
{
    R2D_State *slot;

    if (machine == 0 || machine->count >= R2D_STATE_STACK_MAX) {
        return false;
    }

    slot = &machine->stack[machine->count++];
    *slot = state;
    R2D_StateEnter(machine, slot);
    return true;
}

bool R2D_StateMachinePop(R2D_StateMachine *machine)
{
    if (machine == 0 || machine->count <= 0) {
        return false;
    }

    R2D_StateExit(machine, &machine->stack[machine->count - 1]);
    machine->count--;
    return true;
}

void R2D_StateMachineUpdate(R2D_StateMachine *machine, float dt)
{
    R2D_State *state;

    if (machine == 0 || machine->count <= 0) {
        return;
    }

    state = &machine->stack[machine->count - 1];
    if (state->update != 0) {
        state->update(dt, state->data, machine->user_data);
    }
}

void R2D_StateMachineDraw(R2D_StateMachine *machine)
{
    R2D_State *state;

    if (machine == 0 || machine->count <= 0) {
        return;
    }

    state = &machine->stack[machine->count - 1];
    if (state->draw != 0) {
        state->draw(state->data, machine->user_data);
    }
}

void R2D_StateMachineDrawStack(R2D_StateMachine *machine)
{
    if (machine == 0) {
        return;
    }

    for (int i = 0; i < machine->count; ++i) {
        if (machine->stack[i].draw != 0) {
            machine->stack[i].draw(machine->stack[i].data, machine->user_data);
        }
    }
}

const R2D_State *R2D_StateMachineCurrent(const R2D_StateMachine *machine)
{
    if (machine == 0 || machine->count <= 0) {
        return 0;
    }

    return &machine->stack[machine->count - 1];
}

int R2D_StateMachineCount(const R2D_StateMachine *machine)
{
    return machine != 0 ? machine->count : 0;
}

bool R2D_StateMachineIsEmpty(const R2D_StateMachine *machine)
{
    return machine == 0 || machine->count <= 0;
}

const char *R2D_StateMachineCurrentName(const R2D_StateMachine *machine)
{
    const R2D_State *state = R2D_StateMachineCurrent(machine);

    return state != 0 && state->name != 0 ? state->name : "";
}
