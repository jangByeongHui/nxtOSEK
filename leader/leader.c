/* btslave.c */ 
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

/* global variables */
int speed = 0;
int brake_mode = 0; // 1 : immediately  0 : slowly
int direction = 0;
int steering_count = 0;
int steering_speed = 0;

/* OSEK declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(nxtcar); 
DeclareTask(MoveTask);
//DeclareTask(Initialize);  
DeclareTask(IdleTask);
DeclareTask(SteeringTask);
DeclareTask(BrakeTask);
DeclareTask(HornTask);

DeclareEvent(HornEvent);
DeclareEvent(SteeringEvent);
DeclareEvent(MoveEvent);
DeclareEvent(BrakeEvent);
DeclareResource(R1);
/* below macro enables run-time Bluetooth connection */
#define RUNTIME_CONNECTION

/* LEJOS OSEK hooks */
void ecrobot_device_initialize()
{
  nxt_motor_set_count(NXT_PORT_B, 0);
#ifndef RUNTIME_CONNECTION
  ecrobot_init_bt_slave("LEJOS-OSEK");
#endif
}

void ecrobot_device_terminate()
{
	nxt_motor_set_speed(NXT_PORT_A, 0, brake_mode);
	nxt_motor_set_speed(NXT_PORT_B, 0, brake_mode);
    nxt_motor_set_speed(NXT_PORT_C, 0, brake_mode);
    ecrobot_term_bt_connection();
}

/* LEJOS OSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
  StatusType ercd;

  ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
  if(ercd != E_OK)
  {
    ShutdownOS(ercd);
  }
}

/*
TASK(Initialize)
{
  	GetResource(R1);
	int left_count = 0;
	int right_count = 0;
	display_clear(0);
	display_goto_xy(0,0);
	display_int(nxt_motor_get_count(NXT_PORT_B),0);

	int temp=-1;

	nxt_motor_set_speed(NXT_PORT_B, -15 , 1);

	while(temp!=nxt_motor_get_count(NXT_PORT_B))
	{
		temp=nxt_motor_get_count(NXT_PORT_B);
		systick_wait_ms(50);
	} 

	nxt_motor_set_speed(NXT_PORT_B, 0,1);
	left_count= nxt_motor_get_count(NXT_PORT_B);

	display_goto_xy(0,3);
	display_int(left_count,2);
	display_update();
	nxt_motor_set_count(NXT_PORT_B, 0);
	temp=-1;
	systick_wait_ms(1000);

	nxt_motor_set_speed(NXT_PORT_B, 15 , 1);
	while(temp!=nxt_motor_get_count(NXT_PORT_B))
	{
		temp=nxt_motor_get_count(NXT_PORT_B);
		systick_wait_ms(50);
	} 

	nxt_motor_set_speed(NXT_PORT_B, 0,1);
	right_count= nxt_motor_get_count(NXT_PORT_B);
	
	display_goto_xy(0,5);
	display_int(right_count+left_count,2); //max : 90
	display_update();

  	ReleaseResource(R1);
	TerminateTask();
}
*/


/* nxtcar executed every 5ms */
TASK(nxtcar)
{
  static U8 bt_receive_buf[32]; 
  
  /* read packet data from the master device */  
  ecrobot_read_bt_packet(bt_receive_buf, 32);

  
  if(bt_receive_buf[6]==1)
  {
        brake_mode = 1;
  } 
  else if(bt_receive_buf[6]==2)
  {
	brake_mode = 0;
  }

  if(bt_receive_buf[7] ==1)
  {
	if(brake_mode==1) speed=0;
	else if(speed==0) speed=0;
	else {
		if(bt_receive_buf[3] == 1)	speed+=1;
		else if(bt_receive_buf[3] == 2) speed-=1;
	}
	SetEvent(BrakeTask, BrakeEvent);
  }
  else if(bt_receive_buf[7]==2)
  {
	display_goto_xy(0,7);
	display_string("Horn!!");
	display_update();

	SetEvent(HornTask,HornEvent);
  }else{
	display_clear(1);
  }

  steering_count = nxt_motor_get_count(NXT_PORT_B);
  if (bt_receive_buf[4] == 3) 
  {
	direction = -35;
    if(steering_count<=-30){
		steering_speed = 0;
	}else{
		steering_speed=-60;
	}
  }
  else if (bt_receive_buf[4] == 4) 
  {
	direction = 35;
    if(steering_count>=30){
		steering_speed=0;
	}else{
		steering_speed=60;
	}
  }else{
	if(steering_count>0){
		if(steering_count<5) steering_speed=-10;
		else steering_speed = -50;
	}else if(steering_count<0){
		if(steering_count>-5) steering_speed=10;
		else steering_speed = 50;
	}else if(steering_count==0){
		steering_speed = 0;
		nxt_motor_set_count(NXT_PORT_B, 0);
		steering_count=0;
	}
  }
  SetEvent(SteeringTask, SteeringEvent);


  if(bt_receive_buf[3] == 1 &&bt_receive_buf[7] !=1) // forward
  {
	  if(bt_receive_buf[5] ==1)
	  {
		
		if(bt_receive_buf[4] == 3||bt_receive_buf[4] == 4){
			speed = 40;
		} else{
			speed = 50;
		}
	  }
	  else if(bt_receive_buf[5]==2)
	  {
		if(bt_receive_buf[4] == 3||bt_receive_buf[4] == 4){
			speed = 30;
		} else{
			speed = 40;
		}
	  }
    speed = -speed;
    SetEvent(MoveTask, MoveEvent);
  }
  else if(bt_receive_buf[3] == 2 &&bt_receive_buf[7] !=1) // backward
  {
	  if(bt_receive_buf[5] ==1)
	  {
		speed = 35;
	  }
	  else if(bt_receive_buf[5]==2)
	  {
		speed = 25;
	  }
      speed = speed;
      SetEvent(MoveTask, MoveEvent);
  }


  TerminateTask();
}

TASK(BrakeTask)
{
	while(1)
	{
		WaitEvent(BrakeEvent);
		ClearEvent(BrakeEvent);
		nxt_motor_set_speed(NXT_PORT_A, speed, brake_mode);
		nxt_motor_set_speed(NXT_PORT_B, speed, brake_mode);
		nxt_motor_set_speed(NXT_PORT_C, speed, brake_mode);
	}
}
/* Move Task */
TASK(MoveTask)
{
  while(1)
  {
    WaitEvent(MoveEvent);
    ClearEvent(MoveEvent);

    nxt_motor_set_speed(NXT_PORT_A, speed, 1);
    nxt_motor_set_speed(NXT_PORT_C, speed, 1);

  }  
}

TASK(SteeringTask)
{
	while(1)
	{
		WaitEvent(SteeringEvent);
		ClearEvent(SteeringEvent);
		display_goto_xy(0, 3);
		display_int(steering_count,3);
		display_update();
		nxt_motor_set_speed(NXT_PORT_B, steering_speed, 1);	
	}
}

TASK(HornTask)
{
	while(1)
	{
		WaitEvent(HornEvent);
		ClearEvent(HornEvent);
		int res=ecrobot_sound_tone (660,50, 80);
		if(res==1){		
			display_goto_xy(9,7);
			display_string("ok");
			display_update();		
			systick_wait_ms(500);
		}
	}

}
/* IdleTask */
TASK(IdleTask)
{
  static SINT bt_status = BT_NO_INIT;
	//ActivateTask(Initialize);

  while(1)
  {  
#ifdef RUNTIME_CONNECTION
    ecrobot_init_bt_slave("LEJOS-OSEK");
#endif
	  
    if (ecrobot_get_bt_status() == BT_STREAM && bt_status != BT_STREAM)
    {
      display_clear(0);
      display_goto_xy(0, 0);
      display_string("[BT]");
	  display_goto_xy(0, 3);
	  display_int(steering_count,3);
      display_update();
		
    }
    bt_status = ecrobot_get_bt_status();
  }	
}
