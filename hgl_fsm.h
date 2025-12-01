
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_fsm.h implements a simple finite state machine.
 *
 *
 * USAGE:
 *
 * Include hgl_fsm.h file like this:
 *
 *     #define HGL_FSM_IMPLEMENTATION
 *     #include "hgl_fsm.h"
 *
 * As usual, HGL_FSM_IMPLEMENTATION must not be definied in more than one translation unit.
 *
 * Each transition in the hgl_fsm.h internal state machine has some optionally defined
 * private data. The type of the private data is `void *` by default, but may be changed
 * be defining HGL_FSM_PRIVATE_DATA_T.
 *
 * A user of hgl_fsm.h must implement their own transition table, where each state and event
 * will typcially be instances of some enum. hgl_fsm.h assumes that the underlying type of
 * these enums fit inside an unsigned 32-bit integers. If, for some reason, wish to change
 * this assumption it can be done by defining HGL_FSM_ENUM_BACKING_T.
 *
 *
 * EXAMPLE:
 *
 *     #define HGL_FSM_IMPLEMENTATION
 *     #define HGL_FSM_PRIVATE_DATA_T const char *
 *     #include "hgl_fsm.h"
 *
 *     #include <stdio.h>
 *
 *     typedef enum
 *     {
 *         NIL_STATE = 0, // STRICTLY NECESSARY!!!
 *         STATE_A,
 *         STATE_B,
 *         STATE_C,
 *         N_STATES,
 *     } State;
 *
 *     typedef enum
 *     {
 *         EVENT_1,
 *         EVENT_2,
 *         EVENT_3,
 *         EVENT_4,
 *         N_EVENTS,
 *     } Event;
 *
 *     static const HglFsmTransition transition_table[N_STATES][N_EVENTS] = {
 *         [STATE_A] = {
 *             [EVENT_1] = {.next_state = STATE_B, .private_data = "A-->B\n"},
 *             [EVENT_3] = {.next_state = STATE_C, .private_data = "A-->C\n"},
 *         },
 *         [STATE_B] = {
 *             [EVENT_2] = {.next_state = STATE_C, .private_data = "B-->C\n"},
 *         },
 *         [STATE_C] = {
 *             [EVENT_4] = {.next_state = STATE_A, .private_data = "C-->A\n"},
 *         },
 *     };
 *
 *     int main()
 *     {
 *         HglFsm fsm = hgl_fsm_make(STATE_A, N_EVENTS, &transition_table[0][0]);
 *
 *         while (true) {
 *             Event e = get_next_event_in_some_way();
 *             const char *my_private_data = hgl_fsm_process_event(&fsm, e);
 *             if (hgl_fsm_current_state(&fsm) == NIL_STATE) {
 *                 // Erroneous transition
 *             }
 *             printf("%s", my_private_data);
 *         }
 *
 *         // Erroneous transition
 *     }
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_FSM_H
#define HGL_FSM_H

/*--- Include files ---------------------------------------------------------------------*/

#include <sys/types.h>
#include <stdint.h>

/*--- Public macros ---------------------------------------------------------------------*/

#ifndef HGL_FSM_ENUM_BACKING_T
#  define HGL_FSM_ENUM_BACKING_T uint32_t
#endif
#ifndef HGL_FSM_PRIVATE_DATA_T
#  define HGL_FSM_PRIVATE_DATA_T void *
#endif

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    HGL_FSM_PRIVATE_DATA_T private_data;
    HGL_FSM_ENUM_BACKING_T next_state;
} HglFsmTransition;

typedef struct
{
    HGL_FSM_ENUM_BACKING_T current_state;
    const size_t n_events;
    const HglFsmTransition *transition_table;
} HglFsm;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

HglFsm hgl_fsm_make(HGL_FSM_ENUM_BACKING_T initial_state,
                    size_t n_events,
                    const HglFsmTransition *transition_table);
HGL_FSM_PRIVATE_DATA_T hgl_fsm_process_event(HglFsm *fsm, HGL_FSM_ENUM_BACKING_T event);
HGL_FSM_ENUM_BACKING_T hgl_fsm_current_state(HglFsm *fsm);

#endif /* HGL_FSM_H */

#ifdef HGL_FSM_IMPLEMENTATION

HglFsm hgl_fsm_make(HGL_FSM_ENUM_BACKING_T initial_state,
                    size_t n_events,
                    const HglFsmTransition *transition_table)
{
    return (HglFsm) {
        .current_state    = initial_state,
        .n_events         = n_events,
        .transition_table = transition_table,
    };
}

HGL_FSM_PRIVATE_DATA_T hgl_fsm_process_event(HglFsm *fsm, HGL_FSM_ENUM_BACKING_T event)
{
    uint32_t idx = ((HGL_FSM_ENUM_BACKING_T)fsm->current_state)*fsm->n_events +
                   (HGL_FSM_ENUM_BACKING_T)event;
    const HglFsmTransition *t = &fsm->transition_table[idx];
    fsm->current_state = t->next_state;
    return t->private_data;
}

HGL_FSM_ENUM_BACKING_T hgl_fsm_current_state(HglFsm *fsm)
{
    return fsm->current_state;
}

#endif /* HGL_FSM_IMPLEMENTATION */

