#!/bin/bash
# Script to run the menuconfig
# Matt Geib

source paths.sh

make -C buildroot menuconfig BR2_EXTERNAL=${BUILDROOT_EXTERNAL} BR2_DEFCONFIG=${ALPAQA_DEFCONFIG}
