#include <stdint.h>
#include <stdio.h>
#include <common.h>
#include <nrf_gpio.h>
#include <boards.h>
#include <common.h>
#include <ble_debug_assert_handler.h>


/**
 * Common fault/error handling module.
 *
 * The current implementation is simple, it will just halt the system if any ASSERT() or 
 * any runtime error is detected. 
 */

/**@brief Function for error handling, which is called when an error has occurred.
 *  
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 * 
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    static volatile int recursive_error = 0;

    // on recursive panic/assert, just reset the device.
    if(recursive_error) {
        NVIC_SystemReset();
    }
    recursive_error++;
    printf("App Error %#lx at %s %ld\n", error_code, p_file_name, line_num);

    blink_assert_led();

#ifdef DEBUG
    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    ble_debug_assert_handler(error_code, line_num, p_file_name);
#endif

    // On assert, the system can only recover with a reset.
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

