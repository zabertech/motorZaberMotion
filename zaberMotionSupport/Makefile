TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

ifndef ZML_HOST_ARCH
$(error "motorZaberMotion: Unsupported architecture: $(EPICS_HOST_ARCH)")
endif

ZML_VERSION     = 8.3.0
ZML_SUPPORT_DIR = $(TOP)/zaberMotionSupport/ZaberMotionCppSupport
ZML_ZIP         = $(TOP)/zaberMotionSupport/ZaberMotionCppSupport-$(ZML_VERSION).zip
ZML_URL         = https://www.zaber.com/support/software-downloads.php?product=zml_cpp_support&version=$(ZML_VERSION)

# Download Zaber Motion Library C++ support package if not present
$(if $(wildcard $(ZML_SUPPORT_DIR)),,$(shell \
    echo "Downloading Zaber Motion Library $(ZML_VERSION) C++ support package..." >&2 && \
    curl -fL -o "$(ZML_ZIP)" "$(ZML_URL)" && \
    unzip -q -o "$(ZML_ZIP)" -d "$(TOP)/zaberMotionSupport" && \
    rm -f "$(ZML_ZIP)" || \
    (echo "ERROR: ZML download failed. Run 'make download-zml-support' to retry." >&2 ; \
     rm -f "$(ZML_ZIP)" 2>/dev/null)))

LIB_INSTALLS += $(wildcard $(ZML_SUPPORT_DIR)/lib/$(ZML_HOST_ARCH)/*)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

.PHONY: download-zml-support clean-zml-support

download-zml-support:
	curl -fL -o "$(ZML_ZIP)" "$(ZML_URL)"
	unzip -q -o "$(ZML_ZIP)" -d "$(TOP)/zaberMotionSupport"
	rm -f "$(ZML_ZIP)"

clean-zml-support:
	rm -rf "$(ZML_SUPPORT_DIR)"
