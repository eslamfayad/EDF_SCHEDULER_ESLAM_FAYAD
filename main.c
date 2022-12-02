/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
#include "queue.h"
/****************************************************************
Tasks Handler 
*****************************************************************/
TaskHandle_t Task_Button_1_Monitor_Handler           = NULL;
TaskHandle_t Task_Button_2_Monitor_Handler           = NULL;
TaskHandle_t Task_Periodic_Transmitter_Handler       = NULL;
TaskHandle_t Task_Uart_Receiver_Handler              = NULL;
TaskHandle_t Task_Load_1_Simulation_Handler          = NULL;
TaskHandle_t Task_Load_2_Simulation_Handler          = NULL;
/****************************************************************
Data for real time analysis
*****************************************************************/
/* Task_Button_1_Monitor */
int Task_Button_1_Monitor_Time_In = 0;
int Task_Button_1_Monitor_Time_Out = 0;
int Task_Button_1_Monitor_Total_Time = 0;
/*-------------*/
/* Task_Button_2_Monitor */
int Task_Button_2_Monitor_Time_In = 0;
int Task_Button_2_Monitor_Time_Out = 0;
int Task_Button_2_Monitor_Total_Time = 0;
/*-------------*/
/* Task_Periodic_Transmitter */
int Task_Periodic_Transmitter_Time_In = 0;
int Task_Periodic_Transmitter_Time_Out = 0;
int Task_Periodic_Transmitter_Total_Time = 0;
/*-------------*/
/* Task_Uart_Receiver */
int Task_Uart_Receiver_Time_In = 0;
int Task_Uart_Receiver_Time_Out = 0;
int Task_Uart_Receiver_Total_Time = 0;
/*-------------*/
/* Task_Load_1_Simulation */
int Task_Load_1_Simulation_Time_In = 0;
int Task_Load_1_Simulation_Time_Out = 0;
int Task_Load_1_Simulation_Total_Time = 0;
/*-------------*/
/* Task_Load_2_Simulation */
int Task_Load_2_Simulation_Time_In = 0;
int Task_Load_2_Simulation_Time_Out = 0;
int Task_Load_2_Simulation_Total_Time = 0;
/*-------------*/
 int System_Time = 0;
 int CPU_Load    = 0 ;
 
/*************************************************************/
extern unsigned char txDataSizeToSend;
extern unsigned char txDataSizeLeftToSend;

/*-----------------------------------------------------------*/
typedef struct Message
{
   int MessageID;
   char MessageData[ 20 ] ;
} Message_Type;

Message_Type  Message;

QueueHandle_t  xQueue = NULL;
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/****************************************************************************
/* Task to be created. 
****************************************************************************/
void Button1_Task ( void * pvParameters )
{
     TickType_t xLastWakeTime;
	 Message_Type  Button_1_Signal ;
	 unsigned char button_1_CurrentState  = 1;
     unsigned char button_1_previousState = 1;
	 Button_1_Signal.MessageID =0;
	 xLastWakeTime = xTaskGetTickCount();
    for( ;; )
    {
			 button_1_CurrentState = GPIO_read(PORT_0,PIN0);
			if(button_1_CurrentState != button_1_previousState)
			{
				if(button_1_previousState)
				{
					strcpy( Button_1_Signal.MessageData, "  FALLING" );

				}
				else
			    {
					strcpy( Button_1_Signal.MessageData, "  RAISING" );
				}
               xQueueSend( /* The handle of the queue. */
               xQueue,
               /* The address of the xMessage variable.  sizeof( struct AMessage )
               bytes are copied from here into the queue. */
               ( void * ) &Button_1_Signal,
               /* Block time of 0 says don't block if the queue is already full.
               Check the value returned by xQueueSend() to know if the message
               was sent to the queue successfully. */
               ( TickType_t ) 0 );
			}
			button_1_previousState = button_1_CurrentState ;
		  //GPIO_write(PORT_0, PIN2, PIN_IS_LOW);
            vTaskDelayUntil( &xLastWakeTime, 50 );
	      //GPIO_write(PORT_0, PIN2, PIN_IS_HIGH);
			
	}
}

/****************************************************************************
/* Task to be created. 
****************************************************************************/
void Button2_Task ( void * pvParameters )
{
     TickType_t xLastWakeTime;
	 Message_Type  Button_2_Signal ;
	 unsigned char button_2_CurrentState	 = 1;
     unsigned char button_2_previousState    = 1;
	 Button_2_Signal.MessageID =1;
	 
	 xLastWakeTime = xTaskGetTickCount();
    for( ;; )
    {
			 button_2_CurrentState = GPIO_read(PORT_0,PIN1);
			if(button_2_CurrentState != button_2_previousState)
			{
				if(button_2_previousState)
				{
					strcpy( Button_2_Signal.MessageData, "  FALLING" );

				}
				else
				{
					strcpy( Button_2_Signal.MessageData, "  RAISING" );
				}
               xQueueSend( /* The handle of the queue. */
               xQueue,
               /* The address of the xMessage variable.  sizeof( struct AMessage )
               bytes are copied from here into the queue. */
               ( void * ) &Button_2_Signal,
               /* Block time of 0 says don't block if the queue is already full.
               Check the value returned by xQueueSend() to know if the message
               was sent to the queue successfully. */
               ( TickType_t ) 0 );
			}
			button_2_previousState = button_2_CurrentState ;
		  //GPIO_write(PORT_0, PIN3, PIN_IS_LOW);
            vTaskDelayUntil( &xLastWakeTime, 50 );
	      //GPIO_write(PORT_0, PIN3, PIN_IS_HIGH);
			
	}
}
/****************************************************************************
/* Task to be created. 
****************************************************************************/
void Periodic_Transmitter ( void * pvParameters )
{
     TickType_t xLastWakeTime;
     Message_Type Transmit_Signal ;
	 Transmit_Signal.MessageID =2;	
	 strcpy( Transmit_Signal.MessageData, "  TRANSMIT" );
	 xLastWakeTime = xTaskGetTickCount();
  
    for( ;; )
    {
               xQueueSend( /* The handle of the queue. */
               xQueue,
               /* The address of the xMessage variable.  sizeof( struct AMessage )
               bytes are copied from here into the queue. */
               ( void * ) &Transmit_Signal,
               /* Block time of 0 says don't block if the queue is already full.
               Check the value returned by xQueueSend() to know if the message
               was sent to the queue successfully. */
               ( TickType_t ) 0 );
			   
			 //GPIO_write(PORT_0, PIN4, PIN_IS_LOW);
               vTaskDelayUntil( &xLastWakeTime, 100 );
	         //GPIO_write(PORT_0, PIN4, PIN_IS_HIGH);
	}
}
/* Task to be created. */
void Uart_Receiver ( void * pvParameters )
{
	 TickType_t xLastWakeTime;
	 xLastWakeTime = xTaskGetTickCount();

    for( ;; )
    {
		Message_Type ReadMessage;

      if( xQueue != NULL )
        {
      /* Receive a message from the created queue to hold complex struct AMessage
      structure.  Block for 10 ticks if a message is not immediately available.
      The value is read into a struct AMessage variable, so after calling
      xQueueReceive() xRxedStructure will hold a copy of xMessage. */
         if( xQueueReceive( xQueue,
                         &( ReadMessage ),
                         ( TickType_t ) 10 ) == pdPASS )
            {
         /*ReadMessage now contains a copy of xMessage. */
				    if(ReadMessage.MessageID == 2)
				    {
						     vSerialPutString(" PERIODIC_T",11);                         /*waiting to make sure data send */    
							 while(txDataSizeLeftToSend != 0){}
							 xSerialPutChar('\n');
						 	 while(txDataSizeLeftToSend != 0){}                          /*waiting to make sure data send */
						     vSerialPutString(ReadMessage.MessageData,10);
							 while(txDataSizeLeftToSend != 0){}                          /*waiting to make sure data send */
							 xSerialPutChar('\n');
							
                    }
				    else if(ReadMessage.MessageID == 1)
				    {
	                         vSerialPutString(" BUTTON_1",10);
							 while(txDataSizeLeftToSend != 0){}                          /*waiting to make sure data send */
							 xSerialPutChar('\n');
						 	 while(txDataSizeLeftToSend != 0){}                          /*waiting to make sure data send */
						     vSerialPutString(ReadMessage.MessageData,10);
							 while(txDataSizeLeftToSend != 0){}                          /*waiting to make sure data send */
							 xSerialPutChar('\n');
                    }
					else if(ReadMessage.MessageID == 0)
				    {
	                         vSerialPutString(" BUTTON_2",10);
							 while(txDataSizeLeftToSend != 0){}                        /*waiting to make sure data send */
							 xSerialPutChar('\n');
						 	 while(txDataSizeLeftToSend != 0){}                        /*waiting to make sure data send */
						     vSerialPutString(ReadMessage.MessageData,10);
							 while(txDataSizeLeftToSend != 0){}                       /*waiting to make sure data send */
							 xSerialPutChar('\n');
                    }
            }
        }

	//GPIO_write(PORT_0, PIN5, PIN_IS_LOW);
      vTaskDelayUntil( &xLastWakeTime, 20 );
	//GPIO_write(PORT_0, PIN5, PIN_IS_HIGH);
	}
}
/****************************************************************************
/* Task to be created. 
****************************************************************************/
void Load_1_Simulation ( void * pvParameters )
{
	 int16_t i;
	 TickType_t xLastWakeTime;
	 xLastWakeTime = xTaskGetTickCount();
  
    for( ;; )
    {
			for(i=0 ;i < 27200;i++ )                                            /* for 12ms execution time */
			{
				i=i;
			}
			
	  //GPIO_write(PORT_0, PIN6, PIN_IS_LOW);
        vTaskDelayUntil( &xLastWakeTime, 10 );
	  //GPIO_write(PORT_0, PIN6, PIN_IS_HIGH);

    }
}
/****************************************************************************
/* Task to be created. 
****************************************************************************/
void Load_2_Simulation ( void * pvParameters )
{
	 int32_t i;
	 TickType_t xLastWakeTime;
	 xLastWakeTime = xTaskGetTickCount();
  
    for( ;; )
    {
        for(i=0 ;i < 79800;i++ )                                            /* for 12ms execution time */
			{
				i=i;
			}
	  //GPIO_write(PORT_0, PIN7, PIN_IS_LOW);
        vTaskDelayUntil( &xLastWakeTime, 100 );
      //GPIO_write(PORT_0, PIN7, PIN_IS_HIGH);
	}
    
}
/****************************************************************************
/* Task to be created. 
/* implement tick hook
****************************************************************************/

void vApplicationTickHook(void)
{
	GPIO_write(PORT_0, PIN8, PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
}
/****************************************************************************
/* Task to be created. 
/* implement idle hook
****************************************************************************/

void vApplicationIdleHook(void)
{
 int32_t i;
 GPIO_write(PORT_0, PIN9, PIN_IS_HIGH);

     for(i=0 ;i <8000;i++ )                                /* adding this load for logic analyzer drawing */ 
			{
				i=i;
			}
 GPIO_write(PORT_0, PIN9, PIN_IS_LOW);
}
	

/****************************************************************************
/* main function 
****************************************************************************/
int main( void )
 

{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
  xQueue = xQueueCreate(
                         /* The number of items the queue can hold. */
                         10,
                         /* Size of each item is big enough to hold only a
                         pointer. */
                         sizeof( Message ) );
    /* Create Tasks here */
 
xTaskPeriodicCreate(
                    Button1_Task,                           /* Function that implements the task. */
                    "TASK ONE",                                 /* Text name for the task. */
                    100,                                        /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Button_1_Monitor_Handler,             /* Used to pass out the created task's handle. */
                    50						      );            
 /* Create Tasks here */
 xTaskPeriodicCreate(
                    Button2_Task,                           /* Function that implements the task. */
                    "TASK TWO",                                 /* Text name for the task. */
                    100,                                        /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Button_2_Monitor_Handler ,            /* Used to pass out the created task's handle. */	
                    50 ); 					                    /* periodicity to determine task deadline */
xTaskPeriodicCreate(
                    Periodic_Transmitter,                       /* Function that implements the task. */
                    "Periodic_Transmitter",                     /* Text name for the task. */
                    50,                                         /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Periodic_Transmitter_Handler,         /* Used to pass out the created task's handle. */
                    100);                                       /* periodicity to determine task deadline */
														        
xTaskPeriodicCreate(                                            
                    Uart_Receiver,                              /* Function that implements the task. */
                    "Uart_Receiver",                            /* Text name for the task. */
                   50,                                          /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Uart_Receiver_Handler,                /* Used to pass out the created task's handle. */
										20);                                        /* periodicity to determine task deadline */
xTaskPeriodicCreate(                                            
                    Load_1_Simulation,                          /* Function that implements the task. */
                    "Load_1_Simulation",                        /* Text name for the task. */
                    50,                                         /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Load_1_Simulation_Handler,            /* Used to pass out the created task's handle. */
										10);                                        /* periodicity to determine task deadline */
xTaskPeriodicCreate(                                            
                    Load_2_Simulation,                          /* Function that implements the task. */
                    "Load_2_Simulation",                        /* Text name for the task. */
                    50,                                         /* Stack size in words, not bytes. */
                    ( void * ) 0,                               /* Parameter passed into the task. */
                    1,                                          /* Priority at which the task is created. */
                    &Task_Load_2_Simulation_Handler,            /* Used to pass out the created task's handle. */
										10);                                        /* periodicity to determine task deadline */
/* TASKS TAGS*/									
vTaskSetApplicationTaskTag( Task_Button_1_Monitor_Handler, ( void * ) 1 );
vTaskSetApplicationTaskTag( Task_Button_2_Monitor_Handler, ( void * ) 2 );
vTaskSetApplicationTaskTag( Task_Periodic_Transmitter_Handler, ( void * ) 3 );
vTaskSetApplicationTaskTag( Task_Uart_Receiver_Handler, ( void * ) 4 );
vTaskSetApplicationTaskTag( Task_Load_1_Simulation_Handler, ( void * ) 5 );
vTaskSetApplicationTaskTag( Task_Load_2_Simulation_Handler, ( void * ) 6 );
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


