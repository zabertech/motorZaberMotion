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

# ifdef MOTOR_ZABER_MOTION
# ### motorZaberMotion was built outside motor
# DB_INSTALLS += $(MOTOR_ZABER_MOTION)/db/profileMoveController.template
# DB_INSTALLS += $(MOTOR_ZABER_MOTION)/db/profileMoveController_settings.req
# DB_INSTALLS += $(MOTOR_ZABER_MOTION)/db/profileMoveAxis.template
# else
# ### motorZaberMotion was built inside motor/modules
# DB_INSTALLS += $(MOTOR)/db/profileMoveController.template
# DB_INSTALLS += $(MOTOR)/db/profileMoveController_settings.req
# DB_INSTALLS += $(MOTOR)/db/profileMoveAxis.template
# endif

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

