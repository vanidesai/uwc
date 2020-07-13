# Build tahu library
FROM ubuntu:18.04
RUN apt-get update && apt-get install -y git wget build-essential


RUN apt-get update && apt-get install -y libmosquitto-dev \
    &&  ls \
    &&  pwd \
    &&  git clone --branch v0.5.8 https://github.com/eclipse/tahu \
    &&  git clone --single-branch --branch master https://a723339e75df0c7223b7c725a7c808da43fdad45@github.impcloud.net/uwc/UWC-Core.git \
    &&  cp UWC-Core/.cicd/Tahu/tahu.c ./ \
    &&  rm -rf UWC-Core \
    &&  rm -rf tahu/client_libraries/c/src/tahu.c && cp /tahu.c tahu/client_libraries/c/src \
    &&  cd tahu/client_libraries/c \
    &&	make -s

RUN mkdir kw10 ; \
       export http_proxy="http://proxy-chain.intel.com:911"; \
       export https_proxy="http://proxy-chain.intel.com:911"; \
       export all_proxy="http://proxy-chain.intel.com:911"; \
       dpkg --add-architecture i386 ;\
       apt-get update -y ;\
       apt-get install lsb-core -y ;\
       apt-get install libc6:i386 libgcc1:i386 -y ;\
       apt-get install lib32tinfo5 -y ;\
       apt-get install libncursesw:i386 -y ;\
       apt-get install libnuma1 -y ;\

       adduser --disabled-password --no-create-home --gecos "" hadi ;\
       chown -R hadi kw10 ; \
       su hadi -c "echo Hadi is here*******"; \
       wget --no-check-certificate https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW12.3/Linux/kw-server-installer.12.3.0.819.linux64.sh ;\
       chmod 755 kw-server-installer.12.3.0.819.linux64.sh ;\
       su hadi -c "./kw-server-installer.12.3.0.819.linux64.sh --agree -i /kw10 --license-server kwlic.intel.com:7500" ;\
       ls kw10 ;

RUN  ls -la ~
RUN  mkdir .klocwork ;
RUN  git clone --single-branch --branch master https://a723339e75df0c7223b7c725a7c808da43fdad45@github.impcloud.net/uwc/UWC-Core.git ;
RUN  cp UWC-Core/.cicd/Tahu/ltoken ./ ;
RUN  rm -rf UWC-Core ;
RUN  ls -la ;
ADD  ltoken .klocwork/

RUN mv .klocwork ~
RUN ls -la ~/.klocwork

# build sources
RUN cd /tahu/client_libraries/c \
    &&	export LD_LIBRARY_PATH='/tahu/client_libraries/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	/kw10/bin/kwinject make clean all -s
	
RUN cd /tahu/client_libraries/c; /kw10/bin/kwdeploy sync --url  https://klocwork-jf6.devtools.intel.com:8100 ; \
       /kw10/bin/kwbuildproject --verbose --license-host  klocwork02p.elic.intel.com  --license-port 7500 --host klocwork-jf6.devtools.intel.com --port 8100 --ssl --project UWC --tables-directory "tables_UWC" --incremental kwinject.out --log-file test.log ;\
       cp test.log tables_UWC/build.log ;\
       cp parse_errors.log tables_UWC/parse_errors.log ;\
       /kw10/bin/kwadmin --url  https://klocwork-jf6.devtools.intel.com:8100 load UWC tables_UWC;
