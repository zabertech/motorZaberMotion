TOP=../
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

USR_INCLUDES += ZaberMotionCppSupport/lib/include

T_A = $(EPICS_HOST_ARCH)

# Note, the following files can be updated by running (TODO: npm script ??)
ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/win_x64/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/win_x64/zaber-motion.dll

else ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/win_x86/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/win_x86/zaber-motion.dll

else ifeq (linux-x86_64, $(findstring linux-x86_64, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/linux_x64/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/linux_x64/libzaber-motion.so

else ifeq (linux-arm, $(findstring linux-arm, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/linux_arm/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/linux_arm/libzaber-motion.so

else ifeq (linux-aarch64, $(findstring linux-aarch64, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/linux_arm64/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/linux_arm64/libzaber-motion.so

else ifeq (darwin-x86_64, $(findstring darwin-x86_64, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/darwin_x64/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/darwin_x64/libzaber-motion.dylib

else ifeq (darwin-aarch64, $(findstring darwin-aarch64, $(T_A)))
LIB_INSTALLS    += $(wildcard ZaberMotionCppSupport/lib/darwin_arm64/*)
USR_SYS_LIBS   += ZaberMotionCppSupport/lib/darwin_arm64/libzaber-motion.dylib

else
$(error "motorZaberMotion: Unsupported architecture: $(T_A)")

endif

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE