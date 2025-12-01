
#define HGL_FSM_IMPLEMENTATION
#define HGL_FSM_PRIVATE_DATA_T const char *
#include "hgl_fsm.h"

#include <stdio.h>

typedef enum
{
    NIL_STATE = 0,
    STATE_A,
    STATE_B,
    STATE_C,
    N_STATES,
} State;

typedef enum
{
    EVENT_1,
    EVENT_2,
    EVENT_3,
    EVENT_4,
    N_EVENTS,
} Event;

static const HglFsmTransition transition_table[N_STATES][N_EVENTS] = {
    [STATE_A] = {
        [EVENT_1] = {.next_state = STATE_B, .private_data = "A-->B\n"},
        [EVENT_3] = {.next_state = STATE_C, .private_data = "A-->C\n"},
    },
    [STATE_B] = {
        [EVENT_2] = {.next_state = STATE_C, .private_data = "B-->C\n"},
    },
    [STATE_C] = {
        [EVENT_4] = {.next_state = STATE_A, .private_data = "C-->A\n"},
    },
};

int main()
{
    HglFsm fsm = hgl_fsm_make(STATE_A, N_EVENTS, &transition_table[0][0]);

    printf("%s", hgl_fsm_process_event(&fsm, EVENT_1));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_2));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_4));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_2));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_1));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_4));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_1));
    printf("%s", hgl_fsm_process_event(&fsm, EVENT_3));

}
