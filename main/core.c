#include "core.h"

#include "protocol.h"

void core_task(void* p) {
    if(p == 0) {
        //throw error or somethink
        
        return;
    }
    core_task_init_data* init_data = (core_task_init_data*)p;
    state_t* state = init_data->state;
    QueueHandle_t* sniffer_fifo = init_data->sniffer_fifo;
    //allocated in main()
    free(init_data);

    typedef union {
        controls_t controls;
        pwmControls_t pwmControls;
    } controls_t;
    controls_t controls;
    bool pwmRaw = false;
    bool initialized = false;

    while(true) {
        if(xSemaphoreTake(state->mutex, 0)) {
            //recive data
            sniffer_data data;
            if(xQueueReceive(*sniffer_fifo, &data, 0)) {
                header_t* header = (header_t*)data.buf;

                data.buf += sizeof(header_t);
                data.len -= sizeof(header_t);

                int err = decode_and_handle_packet(state, header, data.buf, data.len);
                if(err != 0) {
                    printf("error while decoding and hadling packet: %d\n", err);
                }
            }

            pwmRaw = state->pwmRaw;
            if(pwmRaw) {
                controls.pwmControls = state->pwmControls;
            } else {
                controls.controls = state->controls;
            }

            initialized = true;
            
            //give mutex back
            xSemaphoreGive(state->mutex);
        }

        if(!initialized) {
            continue;
        }

        if(pwmRaw) {
            setPWMMotor(RIGHT_FRONT, controls.pwmControls.duty0);
            setPWMMotor(LEFT_FRONT, controls.pwmControls.duty1);
            setPWMMotor(RIGHT_BACK, controls.pwmControls.duty2);
            setPWMMotor(LEFT_BACK, controls.pwmControls.duty3);
        }

        //handle gyroscope and shit like this
    }
}