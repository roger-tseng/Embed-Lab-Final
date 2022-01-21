#include "mbed.h"
 
//Number of dutycycle steps for output wave
#define SINE_STEPS        32
//Frequency of output sine in Hz
#define SINE_OUT_FREQ     1000
 
//Constants to compute the sine waveform
#define PI                 3.141592f
#define SINE_STEPS_RAD    (2.0f * PI / (float)SINE_STEPS)
 
//Table to generate the sine waveform using dutycycles
float sine_duty[SINE_STEPS];
 
 
//Frequency of Pulse Width Modulated signal in Hz
#define PWM_FREQ          200000
 
//PWM pin
PwmOut led(PWM_OUT);       //might have to change PwmPin (p22); 
 
//Heartbeat LED
DigitalOut myled(LED1);     //
 
 
//Ticker to update the PWM dutycycle
Ticker pwm_ticker;
 
//Ticker calls this fucntion to update the PWM dutycycle
void pwm_duty_updater() {
  static int idx=0;
  
  led.write(sine_duty[idx]);  // Set the dutycycle % to next value in array //PwmPin
  idx++;                         // Increment the idx
  if (idx == SINE_STEPS) idx=0;  // Reset the idx when teh end has been reached  
 
}
  
int main() {
  int i;
  
  // Init the duty cycle array
  for (i=0; i<SINE_STEPS; i++) {
    sine_duty[i] = ( sin(i * SINE_STEPS_RAD) + 1.0f ) / 2.0f;  // convert sine (-1.0 .. +1.0) into dutycycle (0.0 .. 1.0)
  }  
    
  // Set PWM frequency to 200 KHz (period = 5 us)
  led.period( 1.0f / (float) PWM_FREQ);      //PwmPin
 
  // Init the Ticker to call the dutycyle updater at the required interval
  // The update should be at (SINE_STEPS * SINE_OUT_FREQ) 
  pwm_ticker.attach(&pwm_duty_updater, 1.0f / (float)(SINE_STEPS * SINE_OUT_FREQ));
 
  while(1){ //infinite loop
    // myled = !myled;     //myled 
    ThisThread::sleep_for(0.5);  
   }
       
}