#!../../bin/linux-x86_64/streamtest

## You may have to change streamtest to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/streamtest.dbd"
streamtest_registerRecordDeviceDriver pdbbase

epicsEnvSet("STREAM_PROTOCOL_PATH", ".")

drvAsynIPPortConfigure("NC", "127.0.0.1:6543")
var streamDebug 1


## Load record instances
#dbLoadRecords("db/xxx.db","user=lnls106Host")

dbLoadRecords("db/demo.db")
#dbLoadRecords("db/testwave.db")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=lnls106Host"
