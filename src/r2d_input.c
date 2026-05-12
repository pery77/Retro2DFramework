#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static float R2D_InputClamp01(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }

    if (value > 1.0f) {
        return 1.0f;
    }

    return value;
}

static float R2D_InputBindingValue(const R2D_InputMap *input, const R2D_InputBinding *binding)
{
    const float deadzone = binding->deadzone > 0.0f ? binding->deadzone : input->default_deadzone;
    float axis;
    float value;

    switch (binding->source) {
        case R2D_INPUT_KEY:
            return IsKeyDown(binding->code) ? 1.0f : 0.0f;
        case R2D_INPUT_MOUSE_BUTTON:
            return IsMouseButtonDown(binding->code) ? 1.0f : 0.0f;
        case R2D_INPUT_GAMEPAD_BUTTON:
            return IsGamepadAvailable(input->gamepad) &&
                IsGamepadButtonDown(input->gamepad, binding->code) ? 1.0f : 0.0f;
        case R2D_INPUT_GAMEPAD_AXIS_NEGATIVE:
            if (!IsGamepadAvailable(input->gamepad)) {
                return 0.0f;
            }
            axis = GetGamepadAxisMovement(input->gamepad, binding->code);
            value = axis < -deadzone ? (-axis - deadzone) / (1.0f - deadzone) : 0.0f;
            return R2D_InputClamp01(value);
        case R2D_INPUT_GAMEPAD_AXIS_POSITIVE:
            if (!IsGamepadAvailable(input->gamepad)) {
                return 0.0f;
            }
            axis = GetGamepadAxisMovement(input->gamepad, binding->code);
            value = axis > deadzone ? (axis - deadzone) / (1.0f - deadzone) : 0.0f;
            return R2D_InputClamp01(value);
        default:
            return 0.0f;
    }
}

static R2D_InputAction *R2D_InputActionByName(R2D_InputMap *input, const char *name)
{
    const int index = R2D_InputFindAction(input, name);

    return index >= 0 ? &input->actions[index] : 0;
}

static bool R2D_InputAddBinding(
    R2D_InputMap *input,
    const char *action,
    R2D_InputSource source,
    int code,
    float deadzone
)
{
    R2D_InputAction *item;
    R2D_InputBinding *binding;

    if (input == 0 || action == 0) {
        return false;
    }

    item = R2D_InputActionByName(input, action);
    if (item == 0) {
        const int index = R2D_InputAddAction(input, action);

        if (index < 0) {
            return false;
        }

        item = &input->actions[index];
    }

    if (item->binding_count >= R2D_INPUT_MAX_BINDINGS) {
        return false;
    }

    binding = &item->bindings[item->binding_count++];
    binding->source = source;
    binding->code = code;
    binding->deadzone = deadzone;
    return true;
}

void R2D_InputInit(R2D_InputMap *input)
{
    if (input == 0) {
        return;
    }

    *input = (R2D_InputMap) { 0 };
    input->gamepad = 0;
    input->default_deadzone = 0.25f;
}

void R2D_InputClear(R2D_InputMap *input)
{
    R2D_InputInit(input);
}

void R2D_InputSetGamepad(R2D_InputMap *input, int gamepad)
{
    if (input != 0) {
        input->gamepad = gamepad;
    }
}

void R2D_InputSetDefaultDeadzone(R2D_InputMap *input, float deadzone)
{
    if (input == 0) {
        return;
    }

    if (deadzone < 0.0f) {
        deadzone = 0.0f;
    } else if (deadzone >= 1.0f) {
        deadzone = 0.99f;
    }

    input->default_deadzone = deadzone;
}

int R2D_InputAddAction(R2D_InputMap *input, const char *name)
{
    R2D_InputAction *action;

    if (input == 0 || name == 0 || name[0] == '\0') {
        return -1;
    }

    {
        const int existing = R2D_InputFindAction(input, name);
        if (existing >= 0) {
            return existing;
        }
    }

    if (input->action_count >= R2D_INPUT_MAX_ACTIONS) {
        return -1;
    }

    action = &input->actions[input->action_count];
    *action = (R2D_InputAction) { 0 };
    snprintf(action->name, sizeof(action->name), "%s", name);
    input->action_count++;
    return input->action_count - 1;
}

int R2D_InputFindAction(const R2D_InputMap *input, const char *name)
{
    if (input == 0 || name == 0) {
        return -1;
    }

    for (int i = 0; i < input->action_count; ++i) {
        if (strcmp(input->actions[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

bool R2D_InputBindKey(R2D_InputMap *input, const char *action, int key)
{
    return R2D_InputAddBinding(input, action, R2D_INPUT_KEY, key, 0.0f);
}

bool R2D_InputBindMouseButton(R2D_InputMap *input, const char *action, int button)
{
    return R2D_InputAddBinding(input, action, R2D_INPUT_MOUSE_BUTTON, button, 0.0f);
}

bool R2D_InputBindGamepadButton(R2D_InputMap *input, const char *action, int button)
{
    return R2D_InputAddBinding(input, action, R2D_INPUT_GAMEPAD_BUTTON, button, 0.0f);
}

bool R2D_InputBindGamepadAxis(R2D_InputMap *input, const char *action, int axis, bool positive)
{
    return R2D_InputAddBinding(
        input,
        action,
        positive ? R2D_INPUT_GAMEPAD_AXIS_POSITIVE : R2D_INPUT_GAMEPAD_AXIS_NEGATIVE,
        axis,
        0.0f
    );
}

void R2D_InputUpdate(R2D_InputMap *input)
{
    if (input == 0) {
        return;
    }

    for (int i = 0; i < input->action_count; ++i) {
        R2D_InputAction *action = &input->actions[i];
        float value = 0.0f;

        action->previous_down = action->down;
        action->previous_value = action->value;

        for (int j = 0; j < action->binding_count; ++j) {
            const float binding_value = R2D_InputBindingValue(input, &action->bindings[j]);

            if (binding_value > value) {
                value = binding_value;
            }
        }

        action->value = value;
        action->down = value > 0.0f;
    }
}

bool R2D_InputDown(const R2D_InputMap *input, const char *action)
{
    const int index = R2D_InputFindAction(input, action);

    return index >= 0 ? input->actions[index].down : false;
}

bool R2D_InputPressed(const R2D_InputMap *input, const char *action)
{
    const int index = R2D_InputFindAction(input, action);

    return index >= 0 ? input->actions[index].down && !input->actions[index].previous_down : false;
}

bool R2D_InputReleased(const R2D_InputMap *input, const char *action)
{
    const int index = R2D_InputFindAction(input, action);

    return index >= 0 ? !input->actions[index].down && input->actions[index].previous_down : false;
}

float R2D_InputValue(const R2D_InputMap *input, const char *action)
{
    const int index = R2D_InputFindAction(input, action);

    return index >= 0 ? input->actions[index].value : 0.0f;
}

float R2D_InputAxis(const R2D_InputMap *input, const char *negative_action, const char *positive_action)
{
    return R2D_InputValue(input, positive_action) - R2D_InputValue(input, negative_action);
}
