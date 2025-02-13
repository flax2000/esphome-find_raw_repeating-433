#!/bin/bash
cd `dirname $0`
SCRIPTDIR=`pwd`
cd && source venv/bin/activate 
cd -
esphome logs raw_repeating_433.yaml 
deactivate
read -p "Press any key to continue" x
