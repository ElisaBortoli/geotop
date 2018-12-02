#!/bin/bash

DIR=/home/elisa/Scrivania/MHPC/geotop_3.0/tests/3D/snow_dstr_SENSITIVITY

rm -rf ${DIR}/geotop.inpts ${DIR}/input-meteo ${DIR}/*SE27XX*
rm ${DIR}/output-tabs/* ${DIR}/output-maps/*
cp -r * ${DIR}
