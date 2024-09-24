#!../../bin/linux-x86_64/zaberMotion

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/zaberMotion.dbd"
zaberMotion_registerRecordDeviceDriver pdbbase

cd "${TOP}/iocBoot/${IOC}"

## motorUtil (allstop & alldone)
dbLoadRecords("$(MOTOR)/db/motorUtil.db", "P=zaberMotion:")

## 
< motor.cmd.mmc200

iocInit

## motorUtil (allstop & alldone)
motorUtilInit("zaberMotion:")

# Boot complete
