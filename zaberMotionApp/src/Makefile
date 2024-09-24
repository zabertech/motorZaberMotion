# Makefile
TOP = ../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# 
#USR_CXXFLAGS += -DDEBUG

DBD += zaberMotionSupport.dbd

LIBRARY_IOC_DEFAULT += zaberMotion

SRCS += zaberBaseController.cpp
SRCS += zaberAxis.cpp

zaberMotion_LIBS += motor
zaberMotion_LIBS += asyn
zaberMotion_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
