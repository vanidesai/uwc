#!/bin/bash
################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
################################################################################

export LD_LIBRARY_PATH="/KPIApp/lib/:/usr/local/lib/"
env
./KPIApp > /reports/kpi-tactics/kpi_tactics_status.log 2>&1

# Run GCovr command
gcovr --html -e "../Test" -e ../../bin --exclude-throw-branches -o /reports/kpi-tactics/KPI_reoport.html -r .. .
