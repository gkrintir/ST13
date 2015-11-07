#!/bin/bash

VARS=""

function addVar
{
    VARS=$VARS" "$1"="$2
}

BASEDIR=`pwd`
addVar BASEDIR $BASEDIR
echo $VARS > global-vars.txt

source external/setupCMake.sh
source external/setupSWIG.sh
source external/setupPXL.sh

source external/setupSqlite.sh
source external/setupBoost.sh
source external/setupTheta.sh

#source CMSSW/setupCMSSW.sh  




