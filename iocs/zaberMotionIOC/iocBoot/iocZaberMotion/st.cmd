#!../../bin/linux-aarch64/zaberMotion

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/zaberMotion.dbd"
zaberMotion_registerRecordDeviceDriver pdbbase
dbLoadRecords "db/zaberExampleRecord.db", "user=ubuntu"

## motorUtil (allstop & alldone)
dbLoadRecords("$(MOTOR)/db/motorUtil.db", "P=zaberMotion:")

cd "${TOP}/iocBoot/${IOC}"

## 
< motor.cmd.zaber

iocInit

## motorUtil (allstop & alldone)
motorUtilInit("zaberMotion:")

# Boot complete
