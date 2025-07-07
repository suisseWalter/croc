#!/bin/bash

export PROJ_NAME=croc_custom
export NETLIST=../yosys/out/croc_chip_yosys.v
export TOP_DESIGN=croc_chip
export REPORTS=reports
export SAVE=save
export OUT="out"
export TMP="tmp"
export SV_FLIST="../croc.flist"
git clone https://github.com/suisseWalter/croc.git $PROJ_NAME
cd $PROJ_NAME
git checkout cwa_rsa_seeding
git submodule init
git submodule update --recursive
icdesign ihp13 -update all -nogui
cd yosys/
mkdir tmp
mkdir out
mkdir reports
mkdir save
oseda yosys scripts/yosys_synthesis.tcl 
cd ../openroad/
cp -r -n  ../ihp13/pdk/ihp-sg13g2/libs.ref/sg13g2_sram/* ../technology/
wget https://raw.githubusercontent.com/The-OpenROAD-Project/OpenROAD-flow-scripts/7747f88f70daaeb63f43ce36e71829707b7e3fa7/flow/platforms/ihp-sg13g2/IHP_rcx_patterns.rules
oseda -2025.01 openroad scripts/chip.tcl 
