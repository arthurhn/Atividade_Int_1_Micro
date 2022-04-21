#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/sysctl.c"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/gpio.c"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"

#define red GPIO_PIN_1
#define blue GPIO_PIN_2
#define green GPIO_PIN_3
#define sw_1 GPIO_PIN_4
#define sw_2 GPIO_PIN_0
#define LOW 0x00
#define HIGH 0xFF

uint32_t ui32Period, i=0, tempo_triac, periodo_triac;

void Timer0IntHandler(void)         //pulso rede
{
    GPIOIntEnable(GPIO_PORTF_BASE, sw_1|sw_2);      //debouncer dos switches

    uint8_t Source = GPIOPinRead(GPIO_PORTF_BASE, sw_1|green|blue|red|sw_2);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

        if ((Source & 0x02) == 0x02)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, red, LOW);
        }
        else
            GPIOPinWrite(GPIO_PORTF_BASE, red, red);
}


void Timer1IntHandler(void)         //pulso triac
{
    uint8_t Source = GPIOPinRead(GPIO_PORTF_BASE, sw_1|green|blue|red|sw_2);
    ui32Period = (SysCtlClockGet() / 1)/120 ;
    uint32_t high_triac = ui32Period/10000;
    periodo_triac = i*ui32Period/10 + ui32Period;
    tempo_triac = periodo_triac - ui32Period;
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    if ((Source & 0x04) == 0x04)
            {

                GPIOPinWrite(GPIO_PORTF_BASE, blue, LOW);       //desabilitar, reconfigurar e habilitar

                if(i!=0){
                    TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
                    TimerDisable(TIMER1_BASE, TIMER_A);

                    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
                    TimerLoadSet(TIMER1_BASE, TIMER_A, periodo_triac);
                    IntEnable(INT_TIMER1A);
                    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

                    TimerEnable(TIMER1_BASE, TIMER_A);
                }

            }
    else {
                GPIOPinWrite(GPIO_PORTF_BASE, blue, blue);

               if(i!=0){
                TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
                TimerDisable(TIMER1_BASE, TIMER_A);

                TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
                TimerLoadSet(TIMER1_BASE, TIMER_A, high_triac);
                IntEnable(INT_TIMER1A);
                TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

                TimerEnable(TIMER1_BASE, TIMER_A);
               }
            }
}

void IntPortalF (void){
   uint8_t ui8IntSource;
   ui32Period = (SysCtlClockGet() / 1)/120 ;
   GPIOIntDisable(GPIO_PORTF_BASE, sw_1|sw_2);  //desabilita
   ui8IntSource = GPIOPinRead(GPIO_PORTF_BASE, sw_1|green|blue|red|sw_2);

    if ((ui8IntSource & 0x01) ==  LOW)
   {
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period-1);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER0_BASE, TIMER_A);


    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period-1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER1_BASE, TIMER_A);

        }

    if((ui8IntSource & 0x10) ==  LOW){
                i = i+1;

        }

    if(i>=11)
        i=0;

    GPIOIntClear(GPIO_PORTF_BASE, sw_1|sw_2);

}


int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, red|blue|green);

    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIODirModeSet(GPIO_PORTF_BASE, sw_1|sw_2, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, sw_1|sw_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4,GPIO_FALLING_EDGE);
    IntEnable(INT_GPIOF);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4);
    IntMasterEnable();


    while(1)
        {

        }

}
