# KwadSimSITL
[![Build Status](https://travis-ci.org/timower/KwadSimServer.svg?branch=master)](https://travis-ci.org/timower/KwadSimServer)
[![codecov](https://codecov.io/gh/timower/KwadSimServer/branch/master/graph/badge.svg)](https://codecov.io/gh/timower/KwadSimServer)


The host process that is included in KwadSim.
It hosts the betaflight process and responds to state updates from the KwadSim client.
This allows the betaflight process to be restarted and all static variables to be reset 
without actually restarting the simulator process.
