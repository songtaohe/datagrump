#include <iostream>
#include <math.h>
#include <malloc.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), window_size_float(1.0)

{
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = 5;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return max((unsigned int)floor(window_size_float),(unsigned int)1);
  //return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}


void Controller::ack_received_delay_threshold( const uint64_t sequence_number_acked,
                               /* what sequence number was acknowledged */
                               const uint64_t send_timestamp_acked,
                               /* when the acknowledged datagram was sent (sender's clock) */
                               const uint64_t recv_timestamp_acked,
                               /* when the acknowledged datagram was received (receiver's clock)*/
                               const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  //static uint64_t old_delay = 0;
  uint64_t delay = timestamp_ack_received - send_timestamp_acked;
  double alpha = 30.0;
  double target_delay = 80.0;  
  double recovery_speed = 5.0;

  if(delay > target_delay)
  {
    window_size_float = max(window_size_float / (2.0 - (1.0 + alpha)/(window_size_float + alpha)), 1.0);
    //window_size_float = max(window_size_float / ((target_delay + 2.0*(delay-target_delay))/(target_delay)),1.0);   

//    if(old_delay > target_delay && delay - old_delay > 10) window_size_float = 1.0;


  }
  else
  {
    window_size_float = window_size_float + recovery_speed / window_size_float;
  }
  
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}



double metricA(double* list, int n)
{
  double ret = 0;
  for(int i = 0; i<n-1; i++)
  {
    double jump = list[i+1] - list[i];
    ret += jump > 0 ? jump*jump : 0;
  }

  return ret/(n-1);
}


double metricB(double* list, int n)
{
  double ret = 0;
  for(int i = 0; i<n-1; i++)
  {
    double jump = list[i+1] - list[i];
    ret += jump < 0 ? jump*jump : 0;
  }

  return ret/(n-1);
}

void Controller::ack_received_delay_threshold_varied_target( const uint64_t sequence_number_acked,
                               /* what sequence number was acknowledged */
                               const uint64_t send_timestamp_acked,
                               /* when the acknowledged datagram was sent (sender's clock) */
                               const uint64_t recv_timestamp_acked,
                               /* when the acknowledged datagram was received (receiver's clock)*/
                               const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{ 
  //static uint64_t old_delay = 0;
  uint64_t delay = timestamp_ack_received - send_timestamp_acked;
  static double alpha = 100.0;
  static double target_delay = 80.0; 
  static double recovery_speed = 5.0;
  static double* delay_list = NULL;
  static double* window_list = NULL;
  static int counter = 0;


  static int state = 1; 
  int newstate = 1;
  //double table[3][2] = {{100.0, 10.0},{80.0, 5.0}, {60.0, 1.0 }};
  double table[3][2] = {{80.0, 5.0},{60.0, 3.0}, {40.0, 1.0 }};

  double mA = 0;
  double mB = 0;
  int mW = 32;
  

  if( counter > mW) mA = metricA(delay_list+(counter-mW),mW);
  if( counter > mW) mB = metricB(delay_list+(counter-mW),mW);


  if(mA >70) newstate = 2;

  if(state == 2 && mA < 40) newstate = 1;
  if(state == 0 && mA > 50) newstate = 1;

  if(mA < 30) newstate = 0;

  state = newstate;
  target_delay = table[state][0];
  recovery_speed = table[state][1];

  



  if(delay_list == NULL)
  {
    delay_list = (double*)malloc(sizeof(double)*65536*2);
  }

  if(window_list == NULL)
  {
    window_list = (double*)malloc(sizeof(double)*65536*2);
  }
 



  if(state>-1)
  { 
    if(delay > target_delay)
    { 
      window_size_float = max(window_size_float / (2.0 - (1.0 + alpha)/(window_size_float + alpha)), 1.0);
    }
    else
    { 
      window_size_float = window_size_float + recovery_speed / window_size_float;
    }
  }
  else
  {
    if(delay > target_delay)
    {
      window_size_float = max(window_size_float - 1.0, 1.0); 
    }
    else
    {
      window_size_float = window_size_float + 1.0;
    }

    //if(window_size_float > 70) window_size_float = 70;
  }

  if(state == 2 && window_size_float > 5) window_size_float = 5;
  //if(state == 1 && window_size_float > 50) window_size_float = 50;




  
  printf("HST, %.2f, %lu, %.2f, %.2f, %d\n",window_size_float, timestamp_ack_received - send_timestamp_acked, mA, mB, state*50);

  delay_list[counter] = delay;
  window_list[counter] = window_size_float;  
  counter ++;


 
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
         << endl;
  }



}





double Mean(double * ptr, int n)
{
  double ret = 0;
  for(int i = 0; i< n; i++) ret = ret + ptr[i];
  return ret/n;
}

double Std(double* ptr, int n)
{
  double ret = 0;
  double mean = Mean(ptr,n);
  for(int i = 0; i<n; i++) ret = ret + (ptr[i] - mean)*(ptr[i] - mean);
  return sqrt(ret/n);
}

double dir(double* ptr, int n)
{
  double ret = 0;
  for(int i = 0; i<n-1; i++)
    ret = ret + ptr[i+1] - ptr[i];

  return ret/(n-1);
}

double dir_exp(double* ptr, int n)
{
  double ret = 0;
  double w = 1.0;
  double sum = 0.0;
  double p = 1.2;

  for(int i = 0; i<n-1; i++)
  {
    ret = ret + (ptr[i+1]-ptr[i])*w;
    sum = sum + w;
    w = w * p;
  }
  return ret/sum;
}


double dir_power(double* ptr, int n)
{
  double ret = 0;
  double w = 1.0;
  double sum = 0.0;
  double p = 0.1;

  for(int i = 0; i<n-1; i++)
  {
    ret = ret + (ptr[i+1]-ptr[i])*w;
    sum = sum + w;
    w = w + p;
  }
  return ret/sum;
}




void Controller::ack_received_prediction( const uint64_t sequence_number_acked,
                               /* what sequence number was acknowledged */
                               const uint64_t send_timestamp_acked,
                               /* when the acknowledged datagram was sent (sender's clock) */
                               const uint64_t recv_timestamp_acked,
                               /* when the acknowledged datagram was received (receiver's clock)*/
                               const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  //static uint64_t old_delay = 0;
  uint64_t delay = timestamp_ack_received - send_timestamp_acked;

  static double* delay_list = NULL;
  static double* window_list = NULL;

  static int counter = 0;


  if(delay_list == NULL)
  {
    delay_list = (double*)malloc(sizeof(double)*65536*2);
  }

  if(window_list == NULL)
  {
    window_list = (double*)malloc(sizeof(double)*65536*2);
  }

  int window = 32;

  double w_old_avg = 0;
  double w_cur_avg = 0;
  double w_target = 0;
  double w_ins = 40;
  
  double d_dir = 0;
  double d_std = 0;
  double d_avg = 0;



  double mean_pf = 0;
  double mean_nf = 0;
  double std_pf = 0;
  double std_nf = 0;

  double feedback_pos = 0;
  double feedback_neg = 0;

  double feedback_dir = 0;
  double feedback_avg = 0;


  double alpha = 0.0;
  double beta = 0.14;

  //if(delay > 100) alpha = 0.4;
  //else alpha = 0.0;


  if(counter>window*2)
  {
    w_old_avg = Mean(window_list + counter - window*2, window);
    w_cur_avg = Mean(window_list + counter - window, window);
    
    d_dir = dir_power(delay_list + counter - window, window);
    d_std = Std(delay_list + counter - window, window);
    d_avg = Mean(delay_list + counter - window, window);

    double mean_tp = 75.0;

    mean_pf = min(max((mean_tp*2-d_avg)/mean_tp,0.0),10.0);
    mean_nf = 1.0 + max((d_avg-mean_tp)/mean_tp,-1.0);

    std_pf = min(max((40.0 - d_std) / 20.0, 0.0),10.0);
    std_nf = 1.0 + max((d_std - 20.0) / 20.0, 0.0);

    feedback_pos = mean_pf * std_pf;
    feedback_neg = mean_nf * std_nf;

    feedback_dir = -max(d_dir,0.0) * feedback_neg - min(d_dir,0.0) * feedback_pos; 
    feedback_avg = -max(d_avg - 72.0, 0.0) * feedback_neg - min(d_avg - 68.0, 0.0) * feedback_pos; 

    w_target = max(w_old_avg + alpha * feedback_dir + beta * feedback_avg, 0.0);
    //w_ins = max(w_target * (window + 1) - w_cur_avg * window, 0.0);
    w_ins = w_target;
  }






  printf("HST, %.2f, %lu, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",window_size_float, timestamp_ack_received - send_timestamp_acked, w_old_avg, w_cur_avg, w_target, w_ins,d_dir, d_std, d_avg, feedback_dir, feedback_avg);



  delay_list[counter] = delay;
  window_list[counter] = w_ins;
  counter ++;


  //if(delay > 180) w_ins = 0;
  //if(d_dir > 3) w_ins = 0;

  window_size_float = w_ins;



  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
         << endl;
  }


}






/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  /* Simple AIMD */
  /*
  static uint64_t last_sequence_number_acked = 0;

  if(sequence_number_acked != last_sequence_number_acked + 1)
  {
    window_size_float = max(window_size_float / 2, 1.0f); 
  }
  else
  {
    window_size_float = window_size_float + 1.0 / window_size_float;
  }

  last_sequence_number_acked = sequence_number_acked;

  printf("HST, %.2f, %lu\n",window_size_float, timestamp_ack_received - send_timestamp_acked);
  
  */

  /* Delay Threshold */
  

  //ack_received_delay_threshold(sequence_number_acked, send_timestamp_acked, recv_timestamp_acked, timestamp_ack_received);
  //ack_received_delay_threshold_varied_target(sequence_number_acked, send_timestamp_acked, recv_timestamp_acked, timestamp_ack_received);
  ack_received_prediction(sequence_number_acked, send_timestamp_acked, recv_timestamp_acked, timestamp_ack_received);



 







 

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
