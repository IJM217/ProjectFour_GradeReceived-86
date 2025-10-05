//********************************************************************
//*                    EEE2046F C template                           *
//*==================================================================*
//* DATE CREATED: 04/05/2024                                        *
//* MODIFIED:  Israel Masters                                        *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio Code                                *
//* TARGET:        STM32F0                                           *
//*==================================================================*
//* DESCRIPTION:                                                     *
//*                                                                  *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include "stm32f0xx.h"
#include <lcd_stm32f0.c>
#include "lcd_stm32f0.h"
#include <stdint.h>
#include <stm32f051x8.h>

#define TRUE 1
#define FALSE 0
#define DELAY 30000
typedef uint8_t flag_t;

flag_t startFlag = FALSE;
flag_t stopFlag = FALSE;
flag_t lapFlag = FALSE;
flag_t resetFlag = TRUE;

uint8_t lapsec = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t hundredths = 0;
uint8_t laphun = 0;
uint8_t lapmin = 0;

void formatstring(uint8_t minutes, uint8_t seconds, uint8_t hundredths, char *time_str);
void Delay(void);
void initGPIO(void);
void initTIM14(void);
void TIM14_IRQHandler(void);
void display(void);
void convert2BCDASCII(const uint8_t min, const uint8_t sec, const uint8_t hund, char *resultPtr);
void checkPB(void);
void BCD_to_ASCII(uint8_t bcd_value, char *ascii_value);

// main fuction
int main(void)
{

    init_LCD();
    initGPIO();
    initTIM14();

    startFlag, stopFlag, lapFlag = FALSE;
    resetFlag = TRUE;

    for (;;)
    {

        checkPB();
        Delay();
        display();
    }
}

// Format time into string
void formatstring(uint8_t minutes, uint8_t seconds, uint8_t hundredths, char *time_str)
{

    sprintf(time_str, "%02d:%02d.%02d", minutes, seconds, hundredths);
}

// delay function
void Delay(void)
{
    volatile int i;
    for (i = 0; i <= DELAY; i++)
    {
        // do nothing
    }
}

void initGPIO(void)
{
    // Initialize GPIO ports
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Setup the LEDs
    GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    // Turn off the LEDs
    GPIOB->ODR |= GPIO_ODR_3;

    GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0);

    // Setup registers for buttons
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0 | GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR3_0);

    // Setup the buttons
    GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
}

// time function
void TIM14_IRQHandler(void)
{

    TIM14->SR &= ~TIM_SR_UIF;

    hundredths++;

    if (hundredths == 100)
    {
        hundredths = 0;
        seconds++;

        if (seconds == 60)
        {
            seconds = 0;
            minutes++;
        }
    }
}

// initialize time methods
void initTIM14(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14->PSC = 1;
    TIM14->ARR = 39999;
    TIM14->CR1 &= ~TIM_CR1_CEN;
    TIM14->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM14_IRQn);
}

// display function
void display(void)
{

    lcd_command(CLEAR);

    char lap[12];
    char time[12];

    if (lapFlag)
    {
        // Display lap time
        formatstring(lapmin, lapsec, laphun, lap);
        lcd_putstring("Lap Time");
        lcd_command(LINE_TWO);
        lcd_putstring(lap);
    }
    else
    {
        // Display current time
        formatstring(minutes, seconds, hundredths, time);

        if (startFlag == FALSE && lapFlag == FALSE && stopFlag == FALSE && resetFlag == TRUE)
        {
            lcd_putstring("Stop Watch");
            lcd_command(LINE_TWO);
            lcd_putstring("Press SW0...");
            hundredths = 0;
            laphun = 0;
            lapsec = 0;
            seconds = 0;
            lapmin = 0;
            minutes = 0;
        }
        else if (startFlag == TRUE && lapFlag == FALSE && stopFlag == FALSE && resetFlag == FALSE)
        {
            lcd_putstring("Time");
            TIM14->CR1 |= TIM_CR1_CEN;
            lcd_command(LINE_TWO);
            lcd_putstring(time);
        }
        else if (startFlag == TRUE && lapFlag == TRUE && stopFlag == FALSE && resetFlag == FALSE)
        {
            lcd_putstring("Time");
            lcd_command(LINE_TWO);
            lcd_putstring(time);
        }
        else if (startFlag == TRUE && lapFlag == FALSE && stopFlag == TRUE && resetFlag == FALSE)
        {
            lcd_putstring("Time");
            TIM14->CR1 &= ~TIM_CR1_CEN;
            lcd_command(LINE_TWO);
            lcd_putstring(time);
        }
    }
}

// checking buttons function
void checkPB(void)
{
    // Check if button 0 is pressed
    if ((GPIOA->IDR & GPIO_IDR_0) == 0)
    {
        startFlag = TRUE, lapFlag = FALSE, stopFlag = FALSE, resetFlag = FALSE;
        GPIOB->ODR &= 0;
        GPIOB->ODR |= GPIO_ODR_0;
    }

    // Check if button 1 is pressed
    if ((GPIOA->IDR & GPIO_IDR_1) == 0)
    {
        lapFlag = TRUE;
        laphun = hundredths;
        lapsec = seconds;
        lapmin = minutes;
        GPIOB->ODR &= 0;
        GPIOB->ODR |= GPIO_ODR_1;
    }

    // Check if button 2 is pressed
    if ((GPIOA->IDR & GPIO_IDR_2) == 0)
    {
        startFlag = TRUE, lapFlag = FALSE, stopFlag = TRUE, resetFlag = FALSE;
        GPIOB->ODR &= 0;
        GPIOB->ODR |= GPIO_ODR_2;
    }

    // Check if button 3 is pressed
    if ((GPIOA->IDR & GPIO_IDR_3) == 0)
    {
        startFlag = FALSE, lapFlag = FALSE, stopFlag = FALSE, resetFlag = TRUE;
        GPIOB->ODR &= 0;
        GPIOB->ODR |= GPIO_ODR_3;
    }
}

// Function to convert to ASCII
void convert2BCDASCII(const uint8_t min, const uint8_t sec, const uint8_t hund, char *resultPtr)
{
    // Convert to BCD
    uint8_t hundBCD = (hund / 10) << 4 | (hund % 10);
    uint8_t secBCD = (sec / 10) << 4 | (sec % 10);
    uint8_t minBCD = (min / 10) << 4 | (min % 10);

    // Convert BCD values to ASCII characters and store in resultPtr
    BCD_to_ASCII(minBCD, resultPtr);
    resultPtr[2] = ':';
    BCD_to_ASCII(secBCD, resultPtr + 3);
    resultPtr[5] = '.';
    BCD_to_ASCII(hundBCD, resultPtr + 6);
    resultPtr[8] = '\0'; // Null-terminate the string
}

void BCD_to_ASCII(uint8_t bcd_value, char *ascii_value)
{
    uint8_t tens = bcd_value / 10;
    uint8_t ones = bcd_value % 10;
    ascii_value[0] = '0' + tens;
    ascii_value[1] = '0' + ones;
}
