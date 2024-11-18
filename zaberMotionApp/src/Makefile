# Makefile
TOP = ../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# This module must be built with c++17
USR_CXXFLAGS += -std=c++17

LIBRARY_IOC_Linux  += ZaberMotion
LIBRARY_IOC_Darwin += ZaberMotion
LIBRARY_IOC_WIN32  += ZaberMotion # Not tested

SRCS += zaberController.cpp
SRCS += zaberAxis.cpp
SRCS += zaberConnectionManager.cpp
SRCS += zaberMotionRegister.cpp

ZaberMotion_LIBS += motor asyn
ZaberMotion_LIBS += $(EPICS_BASE_IOC_LIBS)

DBD += zaberMotionSupport.dbd

USR_SYS_LIBS += zaber-motion
USR_INCLUDES += -I ../../../zaberMotionSupport/include

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
