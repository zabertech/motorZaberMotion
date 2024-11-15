TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
T_A = $(EPICS_HOST_ARCH)

ifndef ZML_HOST_ARCH
$(error "motorZaberMotion: Unsupported architecture: $(T_A)")
endif

LIB_INSTALLS += $(wildcard ZaberMotionCppSupport/lib/$(ZML_HOST_ARCH)/*)

#=============================

include $(TOP)/configure/RULES_DIRS
#----------------------------------------
#  ADD RULES AFTER THIS LINE
