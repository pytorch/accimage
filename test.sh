#!/bin/bash

. /opt/intel/ipp/bin/ippvars.sh intel64
python setup.py -v clean
python setup.py -v build -f
sudo -H python setup.py -v install
python test.py

