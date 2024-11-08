# Makefile
TOP = ../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# 
#USR_CXXFLAGS += -DDEBUG

LIBRARY_IOC += ZaberMotion

SRCS += zaberController.cpp
SRCS += zaberAxis.cpp
SRCS += zaberConnectionManager.cpp
SRCS += zaberMotionRegister.cpp

ZaberMotion_LIBS += motor asyn
ZaberMotion_LIBS += $(EPICS_BASE_IOC_LIBS)

USR_INCLUDES += -I$(ZML_SUPPORT_INCLUDE)
USR_LDFLAGS  += -L$(ZML_SUPPORT_LIB)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
