#!/bin/csh
#@environment = "LL_JOB=TRUE"
#@initialdir = /u/user0/yshi/apps/diter
#@error = dfieldpar.err
#@output = dfieldpar.out
#@input = /u/user0/yshi/field.cfg
#@wall_clock_limit = 6:00:00
#@class = sp-3hr
#@job_type = parallel
#@requirements = (Adapter == "hps_ip")
#@min_processors = 5
#@notify_user = shi@falcon.cis.temple.edu
#@notification = always
#@queue
tokens
set echo
date
set mhost = `hostname | cut -d. -f1`-hps
# Use LL hostfiles to start remote daemons
cat /tmp/$LOADL_STEP_OWNER.$LOADL_STEP_ID.hostfile | sp2hosts
# Let daemons start
sleep 120
prun field
# Kill all daemons 
kads
echo job done $mhost
