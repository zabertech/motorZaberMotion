#!../../bin/darwin-aarch64/zaberMotion

# supported architectures
##!../../bin/darwin-aarch64/zaberMotion
##!../../bin/linux-aarch64/zaberMotion
##!../../bin/linux-arm/zaberMotion
##!../../bin/linux-x86_64/zaberMotion
##!../../bin/darwin-x86/zaberMotion

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/zaberMotion.dbd"
zaberMotion_registerRecordDeviceDriver pdbbase

## motorUtil (allstop & alldone)
dbLoadRecords("$(MOTOR)/db/motorUtil.db", "P=zaberMotion:")

cd "${TOP}/iocBoot/${IOC}"

##
< dbPath.cmd.zaber
< motor.cmd.zaber

iocInit

## motorUtil (allstop & alldone)
motorUtilInit("zaberMotion:")

# Boot complete
