
##############################################################
#
# ALPAQA_APPLICATION
#
##############################################################
ALPAQA_APPLICATION_SITE = $(TOPDIR)/../alpaqa_application
ALPAQA_APPLICATION_SITE_METHOD = local

define ALPAQA_APPLICATION_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define ALPAQA_APPLICATION_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/alpaqa_app $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/alpaqa_app-start-stop $(TARGET_DIR)/etc/init.d/S99alpaqa_app
endef

$(eval $(generic-package))
