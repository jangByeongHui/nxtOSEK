#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"


/* global variables */
float safeDistance=27;

int distance1[10]={0,};
int distance2[10]={0,};
int distance3[10]={0,};
int current=0;
int count=0;
int mdistance1=0;
int mdistance2=0;
int cdistance=0;
int stop_flag=0; //0 : move, 1:stop

//PID speed
float sKp=4.8;
float sKi=0;
float sKd=0.125;

float sError;
float sErrorPrevious;

double sPControl=0,sIControl=0,sDControl=0;
double Time=0.05;
double sPIDControl;

//PID LR
float lrKp=36;
float lrKi=0;
float lrKd=0.5;

float lrError;
float lrErrorPrevious;


double lrPControl=0,lrIControl=0,lrDControl=0;
double lrPIDControl;

/* OSEK declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(nxtcar); 
DeclareTask(MoveTask); 
DeclareTask(StopTask);


DeclareEvent(MoveEvent);
DeclareEvent(StopEvent);


/* LEJOS OSEK hooks */
void ecrobot_device_initialize()
{
	ecrobot_init_sonar_sensor(NXT_PORT_S1);
	ecrobot_init_sonar_sensor(NXT_PORT_S2);
	//ecrobot_init_sonar_sensor(NXT_PORT_S3);
	for(int i=0;i<10;i++){
		distance1[i]=ecrobot_get_sonar_sensor(NXT_PORT_S1);
		distance2[i]=ecrobot_get_sonar_sensor(NXT_PORT_S2);
		//distance3[i]=ecrobot_get_sonar_sensor(NXT_PORT_S3);
	}
}

void ecrobot_device_terminate()
{	
	nxt_motor_set_speed(NXT_PORT_B, 0, 1);
	nxt_motor_set_speed(NXT_PORT_C, 0, 1);
    	nxt_motor_set_speed(NXT_PORT_A, 0, 1);
	ecrobot_term_sonar_sensor(NXT_PORT_S1);
	ecrobot_term_sonar_sensor(NXT_PORT_S2);
	//ecrobot_term_sonar_sensor(NXT_PORT_S3);
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
int constrain(double value,int low,int high){
	if(value<low){
		return low;
	}else if(value>high){
		return high;
	}else{
		return (int)value;
	}
}
int abs(int a){
	if(a<0){
		return -a;
	}else{
		return a;
	}
}
/* nxtcar executed every 5ms */
TASK(nxtcar)
{
	

	//Low Pass
	for(int i=0;i<9;i++){
		distance1[i]=distance1[i+1];
		distance2[i]=distance2[i+1];
		//distance3[i]=distance3[i+1];
	}
	distance1[9]=ecrobot_get_sonar_sensor(NXT_PORT_S1);
	distance2[9]=ecrobot_get_sonar_sensor(NXT_PORT_S2);

	
	
	//distance3[9]=ecrobot_get_sonar_sensor(NXT_PORT_S3);
	int sum1=0,sum2=0;
	for(int i=0;i<9;i++){
		sum1+=distance1[i];
		sum2+=distance2[i];
		//sum3+=distance3[i];
	}
	sum1+=distance1[9]*1.2;
	sum2+=distance2[9]*1.2;
	//sum3+=distance3[9]*1.2;
	mdistance1=sum1/10;
	mdistance2=sum2/10;
	//cdistance=sum3/10;	

	display_clear(0);
	display_goto_xy(1,0);
	display_string("dis1:");
	display_int(mdistance1,4);
	display_goto_xy(1,2);
	display_string("dis2:");
	display_int(mdistance2,4);
	display_goto_xy(1,3);
	display_string("Sp:");
	display_int((int)sPIDControl,4);
	display_goto_xy(1,4);
	display_string("LR:");
	display_int((int)lrPIDControl,4);
	display_goto_xy(1,5);
	display_string("Count:");
	display_int(count,4);
	display_goto_xy(1,7);
	display_string("Arr1:");
	display_int(distance1[9],4);	
	display_goto_xy(1,8);
	display_string("Arr2:");
	display_int(distance2[9],4);
	display_goto_xy(1,7);
	//display_string("Cdis:");
	//display_int(cdistance,4);	

	display_update();

	//long Distance Stop
	if((80<=mdistance1 && 80<=mdistance2)){
		SetEvent(StopTask,StopEvent);
		TerminateTask();
	}

	//PID Speed
	
	sError=safeDistance-((constrain(mdistance1,0,80)+constrain(mdistance2,0,80))/2)*0.9735;
	sPControl=sKp*sError;
	sIControl+=sKi*sError*0.05;
	sDControl=sKd*(sError-sErrorPrevious)/Time;
	
	sPIDControl=sPControl+sIControl+sDControl;
	sPIDControl=constrain(sPIDControl,-60,70);

	sErrorPrevious=sError;

	if(mdistance1<15 || mdistance2<15){
		if(sPIDControl < 0) sPIDControl -=5; 
	}

	//PID LR
	count=nxt_motor_get_count(NXT_PORT_B);
	lrError=(mdistance1-mdistance2);
	if(abs(mdistance1-mdistance2)<4){
		lrError=-count;
	}else if(mdistance1>mdistance2){
		lrError=35-count;
	}else if(mdistance1<mdistance2){
		lrError=-35-count;
	}

	lrPControl=lrKp*lrError;
	lrIControl+=lrKi*lrError*0.05;
	lrDControl=lrKd*(lrError-lrErrorPrevious)/Time;
	
	lrPIDControl=lrPControl+lrIControl+lrDControl;
	lrPIDControl=constrain(lrPIDControl,-45,45);
	lrErrorPrevious=lrError;
	current=nxt_motor_get_count(NXT_PORT_B);

//	if((distance1[7]<18 && distance2[9]<43)||(distance1[7]<43&&distance2[9]<18)){
	if(distance1[7]<18||distance2[9]<18){
		if((distance1[9] <= distance1[7] && distance1[9]< 18) || (distance2[9]<=distance2[7]&&distance2[9]< 18)){
			if(sPIDControl<0){
				//TODO : move backward
				sPIDControl = -sPIDControl;
			}
		}
		/*else{
			SetEvent(StopTask,StopEvent);
			TerminateTask();
		}*/
	}

	//Cause MoveEvent
	SetEvent(MoveTask,MoveEvent);
	

 	TerminateTask();
}

/* Move Task */
TASK(MoveTask)
{
  while(1)
  {
    	WaitEvent(MoveEvent);
    	ClearEvent(MoveEvent);

	//LR
	int turn = 0; //0 : straight, 1 : right, 2: left;
	int temp_current=nxt_motor_get_count(NXT_PORT_B);
	if(temp_current<35&&lrPIDControl>0){
		//right
		turn = 1;
		nxt_motor_set_speed(NXT_PORT_B,(int)lrPIDControl+5,1);
	} else if(temp_current>-35&&lrPIDControl<0){
		//left
		turn = 2;
		nxt_motor_set_speed(NXT_PORT_B,(int)lrPIDControl-5,1);
	} else{
		turn = 0;
		nxt_motor_set_speed(NXT_PORT_B,0,1);
	}

	if(sPIDControl <= -65) sPIDControl += 10;
	
	//Speed
	else{
		if(turn==0){
			if(sPIDControl>50){
			    	nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl-10, 1); // left motor
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl-10, 1); // right motor
			}else if(sPIDControl<-50){
			    	nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl+10, 1); // left motor
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl+10, 1); // right motor
			}else{
			   	nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl, 1); // left motor
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl, 1); // right motor
			}
		}
		else if(turn ==1){
			if(sPIDControl>0){
				nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl-10,1);
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl, 1);
			}else{
				nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl+10, 1);
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl, 1);
			}

		}else if(turn==2){
			if(sPIDControl>0){
				nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl, 1);
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl-10, 1);
			}else{
				nxt_motor_set_speed(NXT_PORT_A, (int)sPIDControl, 1);
				nxt_motor_set_speed(NXT_PORT_C,(int)sPIDControl+10, 1);
			}
		}
	}
  }  
}

/* Stop Task */
TASK(StopTask)
{
  while(1)
  {
    WaitEvent(StopEvent);
    ClearEvent(StopEvent);
    nxt_motor_set_speed(NXT_PORT_B, 0, 1);
    nxt_motor_set_speed(NXT_PORT_C,0, 1);
    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
  }  
}	

