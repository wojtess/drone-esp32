#include "core.h"

#include "protocol.h"

void core_task(void* p) {
    if(p == 0) {
        //throw error or somethink
        printf("p = 0\n");
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

    printf("Started core_task\n");

    while(true) {
        if(xSemaphoreTake(state->mutex, 0)) {
            //recive data
            sniffer_data data;
            if(xQueueReceive(*sniffer_fifo, &data, 0)) {
                header_t* header = (header_t*)data.buf;

                data.buf += sizeof(header_t);
                data.len -= sizeof(header_t);
                // printf("recived packet: %d\n", header->id);      

                int err = decode_and_handle_packet(state, header, data.buf, data.len);
                if(err != 0) {
                    printf("error while decoding and hadling packet: %d\n", err);
                }
                // printf("buffer to free: %p\n", data.buf_original_pointer);
                free(data.buf_original_pointer);
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
        } else {
            double t = controls.controls.throttle / 100.0f;
            double y = controls.controls.yaw / 90.0f;
            double p = controls.controls.pitch / 90.0f;
            double r = controls.controls.roll / 90.0f;

            // printf("moc: %f\n", (t - p + y - r) / 4.0f);  

            setPWMMotor(RIGHT_FRONT, (t - p + y - r) / 4.0f);//CW
            setPWMMotor(LEFT_FRONT, (t - p - y + r) / 4.0f);//CCW
            setPWMMotor(RIGHT_BACK, (t + p - y - r) / 4.0f);//CCW
            setPWMMotor(LEFT_BACK, (t + p + y + r) / 4.0f);//CW
        }

        //handle gyroscope and shit like this
    }
}