TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

ifndef ZML_HOST_ARCH
$(error "motorZaberMotion: Unsupported architecture: $(EPICS_HOST_ARCH)")
endif

LIB_INSTALLS += $(wildcard ZaberMotionCppSupport/lib/$(ZML_HOST_ARCH)/*)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
