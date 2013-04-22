#experiment for the synergy FT


import os
import time 
import string
import subprocess


def get_process_id(host, process_name):
    out=os.popen("ssh "+host+" ps -a | grep "+process_name).read()
    words = string.split(out) 
    if len(words)!=0:
        return (words[0])
    else :
        return ("")


def check_if_process_done(host, process_name):
    out=os.popen("ssh "+host+" ps -a | grep "+process_name).read()
    if out !="":
        print("process "+process_name+" on host "+host+" still running!")
        return 1
    else:
        print("process "+process_name+" on host "+host+" stopped!")
        return 0 

def get_next_victim(victim_index):
    f = open('~/.sng_hosts', 'r')
    i=0
    for i in [0,victim_index]:
        victim=string.split(f.readline())
        i=i+1             
    return victim




def run_test(matrix_size, granularity, rounds, max_events,MTTF,num_hosts):
    
    #intialize the environement
    path= os.getcwd()
    
    os.system('./killall')
    time.sleep(4)
    os.system("ssh node001 "+path+"/killall")
    time.sleep(4)
    
    os.environ['MAT_SIZE']=str(matrix_size)
    os.environ['G_MAT']=str(granularity)
    os.environ['ROUNDS']=str(rounds)
    os.system("make")
    
    #start the prun process
    subprocess.Popen([r"prun","matrix","-ft"])
    victim_index=num_hosts-1
    
    
    while max_events!=0:
        
        time.sleep(MTTF)        
        victim=get_next_victim(victim_index)  
            
        if ( check_if_process_done("localhost","prun")==0):
            print('stopping the process')
            break
        else :
            print("killing worker with pid= %s on host %s" %(victim[0],victim[1]) ) 
            os.system("ssh %s kill %s" %(victim[0],victim[1]) ) 
                
        max_events= max_events-1
        victim_index=victim_index-1 
    return 













#######MAIN#########



numhosts_array=[2,4,6]
matrix_size=10000
granularity = 400
rounds=10
max_num_events=1
MTTF_array=[50,100,150,200,250,300,350]


#for numhosts in numhosts_array


for MTTF in MTTF_array:
    print("Running the" + str(MTTF)+ " MTTF iteration")
    run_test(matrix_size, granularity, rounds, max_num_events,MTTF,numhosts_array[0])
    while 1:
        print"Waiting for re-initialization"
        time.sleep(10)
        if (check_if_process_done("localhost","prun")==0):
            break
            
    



 





    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    



