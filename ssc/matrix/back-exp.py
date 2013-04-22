#experiment for the synergy FT


import os
import time 
import string
import subprocess


def process_num():
    out=os.popen("ps -a | grep mtwrk").read()
    words = string.split(out) 
    if len(words)!=0:
        return (words[0])
    else :
        return ("")


def check_if_process_done():
    out=os.popen("ps -a | grep mtclnt").read()
    if out !="":
        print("process mtclnt still running!")
        return 0
    else:
        print("process mtclnt stopped!")
        return 1 




numhosts=4
matrix_size=2000
granularity = 400

num_events=1 # number of crashes
interval = 10 # seconds
failure_rate= num_events/interval
worker_name='mtwrk'
master_name='mtclnt'
#checkpoint_interval= 10 #seconds

#os.system('export MAT_SIZE=1000'+str(matrix_size))
#os.system('export G_MAT='+str(granularity))

os.system('./killall')
time.sleep(4)



os.environ['MAT_SIZE']=str(matrix_size)
os.environ['G_MAT']=str(granularity)
os.system("make")

#Syn_run = "prun matrix -ft &"
subprocess.Popen([r"prun","matrix","-ft"])
print("process "+master_name+" still running!")


while 1:

    time.sleep(10)
    if ( check_if_process_done()==1):
        print('stopping the process')
        break
    
    else :
        print("killing worker with pid= " + str(process_num())) 
        os.system("kill " + str(process_num())) 
        #print("replacing the worker ")
        

 





    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    



