#include <stdint.h>
#include "hardware/gpio.h"
#include "bsp/board.h"
#include "keyer.h"

// GPIO pins
#define KEYER_OUT_GPIO 22
#define LEFT_PADDLE_GPIO 17
#define RIGHT_PADDLE_GPIO 16

// State of the keyer
bool keyer_dit;
bool keyer_dah;

// Debounce variables
#define DEBOUNCE_TIME_MS 10
static uint32_t last_dit_change_ms = 0;
static uint32_t last_dah_change_ms = 0;
static bool raw_dit_state = false;
static bool raw_dah_state = false;

// Read the state of the keyer with 10ms debouncing
void keyer_read_paddles(void) {
    // Read the raw GPIO states
    bool new_dit_state = !gpio_get(LEFT_PADDLE_GPIO);
    bool new_dah_state = !gpio_get(RIGHT_PADDLE_GPIO);
    uint32_t current_time = board_millis();
    
    // Handle dit paddle debouncing
    if (new_dit_state != raw_dit_state) {
        // State changed, update timestamp
        raw_dit_state = new_dit_state;
        last_dit_change_ms = current_time;
    } else if (new_dit_state != keyer_dit && 
              (current_time - last_dit_change_ms) >= DEBOUNCE_TIME_MS) {
        // State has been stable for the debounce period, update the actual keyer state
        keyer_dit = new_dit_state;
    }
    
    // Handle dah paddle debouncing
    if (new_dah_state != raw_dah_state) {
        // State changed, update timestamp
        raw_dah_state = new_dah_state;
        last_dah_change_ms = current_time;
    } else if (new_dah_state != keyer_dah && 
              (current_time - last_dah_change_ms) >= DEBOUNCE_TIME_MS) {
        // State has been stable for the debounce period, update the actual keyer state
        keyer_dah = new_dah_state;
    }
    
    // Update LED based on final debounced state
    board_led_write(keyer_dit ^ keyer_dah);
}

// Every 1ms poll the keyer
void keyer_task(void) {
    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) {
        return; // not enough time
    }
    start_ms += interval_ms;
    
    keyer_read_paddles();
}

void keyer_init(void) {
    // Initialise key input
    gpio_init(LEFT_PADDLE_GPIO);
    gpio_set_dir(LEFT_PADDLE_GPIO, GPIO_IN);
    gpio_pull_up(LEFT_PADDLE_GPIO);
    
    gpio_init(RIGHT_PADDLE_GPIO);
    gpio_set_dir(RIGHT_PADDLE_GPIO, GPIO_IN);
    gpio_pull_up(RIGHT_PADDLE_GPIO);

    // Initialize the debounce variables
    raw_dit_state = !gpio_get(LEFT_PADDLE_GPIO);
    raw_dah_state = !gpio_get(RIGHT_PADDLE_GPIO);
    keyer_dit = raw_dit_state;
    keyer_dah = raw_dah_state;
    last_dit_change_ms = board_millis();
    last_dah_change_ms = board_millis();
    
    keyer_read_paddles();
}
