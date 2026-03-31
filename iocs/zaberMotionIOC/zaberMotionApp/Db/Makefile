TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
#DB += xxx.db

# From motor
DB_INSTALLS += $(MOTOR)/db/profileMoveController.template
DB_INSTALLS += $(MOTOR)/db/profileMoveController_settings.req
DB_INSTALLS += $(MOTOR)/db/profileMoveAxis.template
DB_INSTALLS += $(MOTOR)/db/profileMoveAxis_settings.req
DB_INSTALLS += $(MOTOR)/db/profileMoveControllerZaber.template

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

