Current_Dir=$(pwd)
eis_working_dir="$Current_Dir/../build"
# eis_working_dir=$(pwd)
createTestDir()
{
	cd "${eis_working_dir}"
	rm -rf unit_test_reports
	mkdir -p unit_test_reports/modbus-tcp-master
	mkdir -p unit_test_reports/modbus-rtu-master
	mkdir -p unit_test_reports/mqtt-export
	mkdir -p unit_test_reports/scada-rtu
    mkdir -p unit_test_reports/kpi-tactic
	chown -R $SUDO_USER:$SUDO_USER unit_test_reports
	chmod -R 777 unit_test_reports
	cd "${eis_working_dir}"
}

createTestDir