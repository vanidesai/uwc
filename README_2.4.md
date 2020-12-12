Below steps needs to be followed to execute UWC services without SCADA:


1. Follow the README.md of eis-manifests repo to repo init to the recipe uwc.xml from the EII MR: https://gitlab.devtools.intel.com/Indu/IEdgeInsights/eis-manifests/-/merge_requests/37.
2. DO repo sync to get the latest EII & UWC code in right directories. 
(Since UWC MR is not yet merged, request to take the latest from the git branch https://gitlab.devtools.intel.com/Indu/UWC/UWC_New_Design/-/tree/feature/nagdeep/uwc2.4  ).

3. PROVISIONING:
a. For dev_mode, set DEV_MODE=true &DEV_MODE=false in build/.env, respectively.
b. Run, sudo ./provision_eis.sh ../docker-compose.yml

4. Run UWC containers in Dev mode:
a. cd to build/ directory.
b. docker-compose up --build -d  (This builds & runs all UWC services in their containers)
c. Can use MQTT.FX or any other MQTT client to verify teh flow or all 6 operations for RT/Non-RT, read/write,polling operations.


