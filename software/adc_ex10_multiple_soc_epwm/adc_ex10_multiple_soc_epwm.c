// //#############################################################################
// //
// // FILE:   adc_ex10_multiple_soc_epwm.c
// //
// // TITLE:  ADC ePWM Triggering Multiple SOC
// //
// //! \addtogroup driver_example_list
// //! <h1>ADC ePWM Triggering Multiple SOC</h1>
// //!
// //! This example sets up ePWM1 to periodically trigger a set of conversions on
// //! ADCA and ADCC. This example demonstrates multiple ADCs working together
// //! to process of a batch of conversions using the available parallelism
// //! across multiple ADCs.
// //!
// //! ADCA Interrupt ISRs are used to read results of both ADCA and ADCC.
// //!
// //! \b External \b Connections \n
// //!  - A0, A1, A2 and C2, C3, C4 pins should be connected to signals to be
// //!    converted.
// //!
// //! \b Watch \b Variables \n
// //! - \b adcAResult0 - Digital representation of the voltage on pin A0
// //! - \b adcAResult1 - Digital representation of the voltage on pin A1
// //! - \b adcAResult2 - Digital representation of the voltage on pin A2
// //! - \b adcCResult0 - Digital representation of the voltage on pin C2
// //! - \b adcCResult1 - Digital representation of the voltage on pin C3
// //! - \b adcCResult2 - Digital representation of the voltage on pin C4
// //!
// //
// //#############################################################################
// //
// //
// // $Copyright:
// // Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
// //
// // Redistribution and use in source and binary forms, with or without
// // modification, are permitted provided that the following conditions
// // are met:
// //
// //   Redistributions of source code must retain the above copyright
// //   notice, this list of conditions and the following disclaimer.
// //
// //   Redistributions in binary form must reproduce the above copyright
// //   notice, this list of conditions and the following disclaimer in the
// //   documentation and/or other materials provided with the
// //   distribution.
// //
// //   Neither the name of Texas Instruments Incorporated nor the names of
// //   its contributors may be used to endorse or promote products derived
// //   from this software without specific prior written permission.
// //
// // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// // A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// // OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// // SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// // LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// // DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// // THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// // (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// // OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// // $
// //#############################################################################
// //

// //
// // Included Files
// //
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

//
// Globals
//
uint16_t adcAResult0;
uint16_t adcAResult1;
uint16_t adcAResult2;

// #define DCDC_MODE
#define ACDC_MODE

// Current controller PI parameters
#define CURRENT_CTRL_KP 0.302575807147390f // Proportional gain
#define CURRENT_CTRL_KI 0.160320000000000f             // Integral gain
#define CURRENT_CTRL_OFFSET 0.4f           // Controller offset
#define CURRENT_CTRL_OUT_MAX 0.99f         // Maximum output
#define CURRENT_CTRL_OUT_MIN 0.01f         // Minimum output
#define CURRENT_CTRL_INTEGRAL_MAX 100.0f   // Maximum integral term
#define CURRENT_CTRL_INTEGRAL_MIN -100.0f  // Minimum integral term

// Voltage controller PI parameters
#define VOLTAGE_CTRL_KP 0.127298954473516f // Proportional gain
#define VOLTAGE_CTRL_KI 0.00005f          // Integral gain
#define VOLTAGE_CTRL_OUT_MAX 4.0f          // Maximum output
#define VOLTAGE_CTRL_OUT_MIN 0.0f          // Minimum output
#define VOLTAGE_CTRL_INTEGRAL_MAX 4.0f     // Maximum integral term
#define VOLTAGE_CTRL_INTEGRAL_MIN -4.0f    // Minimum integral term

// DCDC Voltage Controller
#define DCDC_CTRL_KP 0.010181916357995f // Proportional gain
#define DCDC_CTRL_KI 0.000008f         // Integral gain
#define DCDC_CTRL_OUT_MAX 0.99f        // Maximum output
#define DCDC_CTRL_OUT_MIN 0.01f        // Minimum output
#define DCDC_CTRL_INTEGRAL_MAX 100.0f  // Maximum integral term
#define DCDC_CTRL_INTEGRAL_MIN -100.0f // Minimum integral term

#define VOUT_TARGET 40.0f // Target output voltage
#define VOUT_OVP 50.0f    // Over Voltage Protection
#define IL_OCP 5.0f       // Over Current Protection

typedef enum
{
    STATE_BROWN_OUT = 0,
    STATE_SOFT_START,
    STATE_CLOSED_LOOP,
    STATE_OVP, // Over Voltage Protection
    STATE_OCP,  // Over Current Protection
    STATE_UVP, // Under Voltage Protection
    // Add more states if needed
} PfcState_t;

static PfcState_t g_currentState = STATE_BROWN_OUT;

volatile float current_Vout_target = 20.0f; // Target output voltage
volatile float Vout;
volatile float Vin;
volatile float Iin;
volatile float duty = 0;
// static uint32_t softStartCounter = 0;

// Add a static variable for minimum time tracking
static uint32_t softStartTimer = 0;
#define SOFT_START_MIN_TIME 0 // Minimum time (count) before next voltage increment

// Add these variables to track relay state and timing
static uint16_t relayOpenTimer = 0;
static bool relayWasClosed = false;

//
// Function Prototypes
//
void initEPWM();
void setPWMDutyCycle(float32_t dutyCycle);
__interrupt void adcA1ISR(void);
void forcePWMHIGH(void);
void forcePWMLOW(void);
void floatToString(float value, char *buffer, int decimalPlaces);

void PfcStateMachineStep(void)
{
    // Track relay state changes
    bool relayCurrentlyOpen = (GPIO_readPin(RLY_CTRL) == 0);
    
    // If the relay just opened, reset the timer
    if (relayWasClosed && relayCurrentlyOpen) {
        relayOpenTimer = 0;
    }
    
    // Increment timer if relay is open
    if (relayCurrentlyOpen) {
        relayOpenTimer++;
    } else {
        // Reset timer if relay is closed
        relayOpenTimer = 0;
    }
    
    // Save current relay state for next comparison
    relayWasClosed = !relayCurrentlyOpen;

    switch (g_currentState)
    {
    case STATE_BROWN_OUT:
        // Check if input voltage is above threshold
        if (Vin > 10.0f && Vout > 18.0f)
        {
            g_currentState = STATE_SOFT_START;
            current_Vout_target = Vout + 1.0f; // Start just above the current voltage
            softStartTimer = 0; // Reset the timer when entering soft start
        }
        GPIO_writePin(RLY_CTRL, 0); // Turn off relay
        break;
    case STATE_SOFT_START:
        // First check for error conditions
        if (Vout > VOUT_OVP)
        {
            g_currentState = STATE_OVP;
            break;
        }
        // Check if output current is above threshold - with delay after relay opens
        // Only check if relay is closed OR relay has been open for more than 40000 ticks
        if (Iin > IL_OCP && (!relayCurrentlyOpen || relayOpenTimer > 2000))
        {
            g_currentState = STATE_OCP;
            break;
        }
        // if (Vin < 10.0f)
        // {
        //     g_currentState = STATE_UVP;
        //     break;
        // }
        
        // Normal soft start behavior continues if no errors
        // Increment the timer
        softStartTimer++;
        
        // Check if both conditions are met:
        // 1. Output voltage is close to target
        // 2. Minimum time has elapsed
        if ((Vout >= (current_Vout_target - 1.0f)) && (softStartTimer >= SOFT_START_MIN_TIME))
        {
            // Increment the target voltage by 1V
            current_Vout_target += 0.1250f;
            // Reset timer for next step
            softStartTimer = 0;
            
            // If target reached or exceeded, move to closed loop mode
            if (current_Vout_target >= VOUT_TARGET)
            {
                current_Vout_target = VOUT_TARGET;
                g_currentState = STATE_CLOSED_LOOP;
            }
        }
        GPIO_writePin(RLY_CTRL, 1); // Turn on relay
        break;
    case STATE_CLOSED_LOOP:
        // Check if output voltage is above threshold
        if (Vout > VOUT_OVP)
        {
            g_currentState = STATE_OVP;
        }
        // Check if output current is above threshold - with delay after relay opens
        // Only check if relay is closed OR relay has been open for more than 10 ticks
        if (Iin > IL_OCP && (!relayCurrentlyOpen || relayOpenTimer > 10))
        {
            g_currentState = STATE_OCP;
        }
        #ifdef DCDC_MODE
        if (Vin < 10.0f)
        {
            g_currentState = STATE_UVP;
        }
        #endif
        GPIO_writePin(RLY_CTRL, 1); // Turn on relay
        break;
    case STATE_OVP:
        // Implement OVP protection
        GPIO_writePin(RLY_CTRL, 0); // Turn off relay
        forcePWMLOW();
        break;
    case STATE_OCP:
        // Implement OCP protection
        GPIO_writePin(RLY_CTRL, 0); // Turn off relay
        forcePWMLOW();
        break;
    case STATE_UVP:
        // Implement UVP protection
        GPIO_writePin(RLY_CTRL, 0); // Turn off relay
        forcePWMLOW();
        break;
    default:
        break;
    }
}
//
// Main
//
void main(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pullups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    // Configure GPIO1 as output for toggling in the ISR
    GPIO_setPinConfig(GPIO_1_GPIO1);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    //
    // Set up the ADCA and ADCC and initialize the ADC SOC.
    // ADC Resolution - 12-bit, signal mode - single ended
    // ADCA SOC0, SOC1, SOC2 are configured to convert A0,
    // A1 and A2 channels with EPWM1SOCA as SOC trigger.
    // ADCC SOC0, SOC1, SOC2 are configured to convert C2,
    // C3, C4 channels with EPWM1SOCA as trigger
    //
    Board_init();

    //
    // Configure EPWM1 ADC SOCA trigger
    //
    initEPWM();

    //
    // Enable ADC interrupt
    //
    Interrupt_enable(INT_ADCA1);

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    char *msg; // Message sent through terminal window
    //
    // Send starting message.
    //
    msg = "\r\n\n\nHello World!\r\n";
    SCI_writeCharArray(mySCI0_BASE, (uint16_t *)msg, strlen(msg));

    //
    // Start ePWM1, enabling SOCA and putting the counter in up-down (center-aligned) mode
    // setPWMDutyCycle(50);
    forcePWMLOW();
    //
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    //
    // Take conversions indefinitely in loop
    //
    do
    {
    } while (1);
}

//
// Function to configure ePWM1 to generate the SOC.
//
void initEPWM(void)
{
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(0, GPIO_QUAL_SYNC);
    // Disable SOCA
    //
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

    //
    // Configure the SOC to occur on the first up-count event when counter equals CMPA
    //
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO_OR_PERIOD);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);

    //
    // Set the compare A value to 500 and the period to 1999
    // For center-aligned PWM, the effective frequency is halved because
    // the counter counts up to period and then back down to 0
    //
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 249);
    EPWM_setTimeBasePeriod(EPWM1_BASE, 499);

    //
    // Configure action qualifier for center-aligned PWM
    // Set output high when counter equals CMPA during up-count
    //
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    //
    // Set output low when counter equals CMPA during down-count
    //
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    //
    // Set the local ePWM module clock divider to /1
    //
    EPWM_setClockPrescaler(EPWM1_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    //
    // Freeze the counter, will be set to up-down mode in main()
    //
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
}

//
// Function to set PWM duty cycle (0-100%)
//
void setPWMDutyCycle(float32_t dutyCycle)
{
    uint16_t period, compareValue;

    // Ensure the duty cycle is within valid range (0-100%)
    if (dutyCycle < 0.0f)
    {
        dutyCycle = 0.0f;
    }
    else if (dutyCycle > 100.0f)
    {
        dutyCycle = 100.0f;
    }

    // Get the current period value
    period = EPWM_getTimeBasePeriod(EPWM1_BASE);

    // Calculate the compare value based on the duty cycle
    // For duty cycle = 0%, compareValue = period
    // For duty cycle = 100%, compareValue = 0
    // This is because the output goes HIGH at CMPA and LOW at zero
    compareValue = (uint16_t)(period - ((dutyCycle / 100.0f) * period));

    // Set the new compare value
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, compareValue);
}

void forcePWMHIGH(void)
{
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 0);
}

void forcePWMLOW(void)
{
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 10000);
}

//
// Function to convert float to string with specified decimal places
//
void floatToString(float value, char *buffer, int decimalPlaces)
{
    // Handle negative numbers
    if (value < 0)
    {
        *buffer++ = '-';
        value = -value;
    }

    // Extract integer part
    int32_t intPart = (int32_t)value;

    // Extract decimal part
    float decPart = value - intPart;

    // Convert integer part to string
    int i = 0;
    char intStr[16]; // Buffer for integer part

    // Handle case when integer part is 0
    if (intPart == 0)
    {
        intStr[i++] = '0';
    }
    else
    {
        // Convert digits from right to left
        while (intPart > 0)
        {
            intStr[i++] = '0' + (intPart % 10);
            intPart /= 10;
        }
    }

    // Reverse the integer string
    int j;
    for (j = 0; j < i / 2; j++)
    {
        char temp = intStr[j];
        intStr[j] = intStr[i - j - 1];
        intStr[i - j - 1] = temp;
    }
    intStr[i] = '\0'; // Null terminate

    // Copy integer part to buffer
    for (j = 0; j < i; j++)
    {
        *buffer++ = intStr[j];
    }

    // Add decimal point
    if (decimalPlaces > 0)
    {
        *buffer++ = '.';

        // Convert decimal part
        while (decimalPlaces > 0)
        {
            decPart *= 10;
            int digit = (int)decPart;
            *buffer++ = '0' + digit;
            decPart -= digit;
            decimalPlaces--;
        }
    }

    *buffer = '\0'; // Null terminate the final string
}

float current_controller(float Iref, float Iactual)
{
    float E = Iref - Iactual;
    static float i = 0;
    float Up = CURRENT_CTRL_KP * E;
    float sum_i = CURRENT_CTRL_KI * E + i;
    if (sum_i > CURRENT_CTRL_INTEGRAL_MAX)
    {
        sum_i = CURRENT_CTRL_INTEGRAL_MAX;
    }
    else if (sum_i < CURRENT_CTRL_INTEGRAL_MIN)
    {
        sum_i = CURRENT_CTRL_INTEGRAL_MIN;
    }
    i = sum_i;
    float res = Up + sum_i + CURRENT_CTRL_OFFSET;
    if (res > CURRENT_CTRL_OUT_MAX)
    {
        res = CURRENT_CTRL_OUT_MAX;
    }
    else if (res <= CURRENT_CTRL_OUT_MIN)
    {
        res = CURRENT_CTRL_OUT_MIN;
    }
    return res;
}

float voltage_controller(float Vref, float Vactual)
{
    float E = Vref - Vactual;
    static float i = 0;
    float Up = VOLTAGE_CTRL_KP * E;
    float sum_i = VOLTAGE_CTRL_KI * E + i;
    if (sum_i > VOLTAGE_CTRL_INTEGRAL_MAX)
    {
        sum_i = VOLTAGE_CTRL_INTEGRAL_MAX;
    }
    else if (sum_i < VOLTAGE_CTRL_INTEGRAL_MIN)
    {
        sum_i = VOLTAGE_CTRL_INTEGRAL_MIN;
    }
    i = sum_i;
    float res = Up + sum_i;
    if (res > VOLTAGE_CTRL_OUT_MAX)
    {
        res = VOLTAGE_CTRL_OUT_MAX;
    }
    else if (res <= VOLTAGE_CTRL_OUT_MIN)
    {
        res = VOLTAGE_CTRL_OUT_MIN;
    }
    return res;
}

float dcdc_controller(float Vref, float Vactual)
{
    float E = Vref - Vactual;
    static float i = 0;
    float Up = DCDC_CTRL_KP * E;
    float sum_i = DCDC_CTRL_KI * E + i;
    if (sum_i > DCDC_CTRL_INTEGRAL_MAX)
    {
        sum_i = DCDC_CTRL_INTEGRAL_MAX;
    }
    else if (sum_i < DCDC_CTRL_INTEGRAL_MIN)
    {
        sum_i = DCDC_CTRL_INTEGRAL_MIN;
    }
    i = sum_i;
    float res = Up + sum_i;
    if (res > DCDC_CTRL_OUT_MAX)
    {
        res = DCDC_CTRL_OUT_MAX;
    }
    else if (res <= DCDC_CTRL_OUT_MIN)
    {
        res = DCDC_CTRL_OUT_MIN;
    }
    return res;
}

//
// ADC A Interrupt 1 ISR
//
int count = 0;
char buffer[150];

// Variables for averaging ADC readings
static uint16_t isr_counter = 0;
static float Vout_sum = 0.0f;
static float Vin_sum = 0.0f;
static float Iin_sum = 0.0f;
static const uint16_t ISR_DIVIDER = 2; // 100kHz / 2 = 50kHz

__interrupt void adcA1ISR(void)
{
    // Toggle GPIO1
    GPIO_togglePin(1);
    //
    // Store results
    //
    adcAResult0 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0); // Vout
    adcAResult1 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1); // Vin
    adcAResult2 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2); // Iin

    // Calculate raw values and accumulate for averaging
    float Vout_current = (float)adcAResult0 * 0.01332894198f;
    float Vin_current = (float)adcAResult1 * 0.01194061887f;
    float Iin_current = (float)adcAResult2 * 0.0007466063348779414f;

    Vout_sum += Vout_current;
    Vin_sum += Vin_current;
    Iin_sum += Iin_current;

    // Increment the counter
    isr_counter++;

    // Execute controller logic at reduced frequency (50kHz)
    if (isr_counter >= ISR_DIVIDER)
    {
        // Calculate averages
        Vout = Vout_sum / ISR_DIVIDER;
        Vin = Vin_sum / ISR_DIVIDER;
        Iin = Iin_sum / ISR_DIVIDER;

        // Reset the sums for next cycle
        Vout_sum = 0.0f;
        Vin_sum = 0.0f;
        Iin_sum = 0.0f;

        // Reset the counter
        isr_counter = 0;

        // Controller logic
        PfcStateMachineStep();
        if (g_currentState == STATE_CLOSED_LOOP || g_currentState == STATE_SOFT_START)
        {
            duty = 0.0f;
            // Calculate the target output voltage
            float Vout_target = current_Vout_target;
            #ifdef DCDC_MODE
            duty = dcdc_controller(Vout_target, Vout);
            #endif

            #ifdef ACDC_MODE
            float I_tar = voltage_controller(Vout_target, Vout) * Vin / 33.9411255f;
            duty = current_controller(I_tar, Iin);
            #endif
        }
        else
        {
            duty = 0.0f;
        }
        setPWMDutyCycle(duty * 100.0f);

        // Optional debugging output (uncomment if needed)

        if (++count % 50000 == 0)
        {
            // Format the values using our custom function
            static char voutStr[20], vinStr[20], IinStr[20], dutyStr[20], cvoutStr[20];
            floatToString(Vout, voutStr, 2);
            floatToString(Vin, vinStr, 2);
            floatToString(Iin, IinStr, 4);
            floatToString(duty, dutyStr, 2);

            // Manually construct the output string without %f
            strcpy(buffer, "Vout: ");
            strcat(buffer, voutStr);
            strcat(buffer, ", Vin: ");
            strcat(buffer, vinStr);
            strcat(buffer, ", Iin: ");
            strcat(buffer, IinStr);
            strcat(buffer, ", Duty: ");
            strcat(buffer, dutyStr);
            strcat(buffer, ", Target: ");
            floatToString(current_Vout_target, cvoutStr, 2);
            strcat(buffer, cvoutStr);
            strcat(buffer, "\r\n");

            // Send the results to the terminal window
            SCI_writeCharArray(mySCI0_BASE, (uint16_t *)buffer, strlen(buffer));
            switch (g_currentState)
            {
            case STATE_BROWN_OUT:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: BROWN_OUT\r\n", strlen("State: BROWN_OUT\r\n"));
                break;
            case STATE_SOFT_START:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: SOFT_START\r\n", strlen("State: SOFT_START "));
                break;
            case STATE_CLOSED_LOOP:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: CLOSED_LOOP\r\n", strlen("State: CLOSED_LOOP\r\n"));
                break;
            case STATE_OVP:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: OVP\r\n", strlen("State: OVP\r\n"));
                break;
            case STATE_OCP:

                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: OCP\r\n", strlen("State: OCP\r\n"));
                break;
            case STATE_UVP:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: UVP\r\n", strlen("State: UVP\r\n"));
                break;
            default:
                SCI_writeCharArray(mySCI0_BASE, (uint16_t *)"State: UNKNOWN\r\n", strlen("State: UNKNOWN\r\n"));
                break;
            }
            count = 0;
        }
    }

    //
    // Clear the interrupt flag
    //
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    //
    // Check if overflow has occurred
    //
    if (true == ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// //
// // End of file
// //
