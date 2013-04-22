/*                        syn-ray.h 

     This is the header file for Ray-tracing program with Synergy. 

     About the Input POV file, X, Y resolutions, picture quality,
     and other parameters for ray tracing will be put in 'povray.def'.

     In this header file, we will only specify the part we need for
     parallel programming such as the data format that we put to
     the tuple space....

     We need 2 tuple spaces,        
  
     A:  job  : we put out job information for each worker. 
     B:  output: we received the output from the worker.

     C FU
*/

#define CHAR unsigned char
#define TERM 9999           /* terminate signal for tuple space */


/* we put job_unit to job tuple space */ 
     
typedef struct job_unit {
   int y1;         /* draw begin y1 */
   int y2;         /* draw end y2 */
  } JOB_UNIT;   /* X1,X2 can be known by 'povray.def' */


 
