#!/bin/bash
# Script to save the external defconfig after using menuconfig
# Matt Geib

source paths.sh

make -C buildroot savedefconfig BR2_EXTERNAL=${BUILDROOT_EXTERNAL} BR2_DEFCONFIG=${ALPAQA_DEFCONFIG}
