#include <msp430.h>

// Function to initialize the potentiometer
void initializePotentiometer() {
    // Configure P1.2 for ADC (analog-to-digital converter)
    P1SEL1 |= BIT2;
    P1SEL0 |= BIT2;
    // Set the selected channel to A2 (P1.2)
    ADCMCTL0 |= ADCINCH_2;
}

// Function to control speed with potentiometer
void controlSpeedWithPotentiometer() {
    // Start ADC conversion
    ADCCTL0 |= ADCENC + ADCSC;

    // Wait for ADC conversion to complete
    while ((ADCIFG & ADCIFG) == 0);

    // Read ADC value and calculate speed percentage
    volatile float ADC_Value = ADCMEM0;
    volatile float speedPercentage = (ADC_Value * 100) / 4095;

    // Adjust the speed control logic as needed
    TB0CCR1 = (int)speedPercentage;  // Update PWM duty cycle based on potentiometer value
}

int main(void) {
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Unlock GPIO
    PM5CTL0 &= ~LOCKLPM5;

    // Configure GPIO for motor control and PWM
    P1DIR |= BIT5 | BIT7 | BIT6 | BIT0;
    P6DIR |= BIT6;
    P1OUT |= BIT5 | BIT7;  // Set initial conditions for motor control
    P1OUT &= ~BIT6;
    P1OUT &= ~BIT0;
    P6OUT &= ~BIT6;
    P1SEL1 |= BIT2;  // Configure P1.2 for PWM
    P1SEL0 |= BIT2;

    // Initialize ADC and PWM
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;
    ADCCTL0 |= ADCON;
    ADCCTL1 |= ADCSSEL_2 | ADCSHP;
    ADCCTL2 &= ~ADCRES;
    ADCCTL2 |= ADCRES_2;
    initializePotentiometer();

    // Configure Timer B0 for PWM
    TB0CTL |= TBCLR;           // Clear Timer B0
    TB0CTL |= TBSSEL__SMCLK;   // Set Timer B0 clock source to SMCLK
    TB0CTL |= MC__UP;          // Set Timer B0 to Up mode
    TB0CCR0 = 100;             // Set Timer B0 period
    TB0CCR1 = 10;              // Set Timer B0 CCR1 value for PWM duty cycle
    TB0CCTL0 |= CCIE;          // Enable Timer B0 CCR0 interrupt
    TB0CCTL1 |= CCIE;          // Enable Timer B0 CCR1 interrupt
    TB0CCTL0 &= ~CCIFG;        // Clear Timer B0 CCR0 interrupt flag
    TB0CCTL1 &= ~CCIFG;        // Clear Timer B0 CCR1 interrupt flag

    // Enable interrupts
    __enable_interrupt();

    // Main loop
    while (1) {
        controlSpeedWithPotentiometer();
    }

    return 0;
}

// ADC ISR
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void) {
    // ADC interrupt service routine
    // (You can leave this empty or add additional logic if needed)
}

// Timer B0 CCR0 ISR
#pragma vector=TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void) {
    // Timer B0 CCR0 interrupt service routine
    // Set motor control pin
    P1OUT |= BIT7;
    TB0CCTL0 &= ~CCIFG;  // Clear Timer B0 CCR0 interrupt flag
}

// Timer B0 CCR1 ISR
#pragma vector=TIMER0_B1_VECTOR
__interrupt void ISR_TB1_CCR1(void) {
    // Timer B0 CCR1 interrupt service routine
    // Clear motor control pin
    P1OUT &= ~BIT7;
    TB0CCTL1 &= ~CCIFG;  // Clear Timer B0 CCR1 interrupt flag
}
