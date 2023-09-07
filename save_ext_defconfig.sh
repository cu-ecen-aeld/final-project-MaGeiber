#!/bin/bash
# Script to save the external defconfig after using menuconfig
# Matt Geib

source paths.sh

make -C buildroot savedefconfig BR2_EXTERNAL=${BUILDROOT_EXTERNAL} BR2_DEFCONFIG=${ALPAQA_DEFCONFIG} BR2_PACKAGE_RPI_FIRMWARE_CONFIG_FILE=${PI_CONFIG_TXT} BR2_PACKAGE_RPI_FIRMWARE_CMDLINE_FILE=${PI_CMDLINE_TXT}
