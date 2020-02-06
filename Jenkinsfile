pipeline {
    agent { label 'rbhe-uwc' }
    options {
        timestamps()
        disableConcurrentBuilds()
    }
    environment {
        // the following options are for the rbheStaticCodeScan block
        // For UWC, we will only leverage checkmarx and protex
        SCANNERS = 'checkmarx,protex'
        PROJECT_NAME = 'UWC'

        SLACK_SUCCESS = '#indu-uwc'
        SLACK_FAIL   = '#indu-uwc'

        PROFILING_MODE = false
        EIS_BRANCH = 'v2.1'
    }
    stages {
        stage('Prep Builder Image') {
            steps {
                script {
                    // Need to install docker-compose
                    // https://docs.docker.com/compose/install/
                    def dockerfile = '''
                    FROM ubuntu:18.04
                    COPY --from=docker:latest /usr/local/bin/docker /usr/local/bin/docker
                    RUN apt-get update && apt-get install -y curl git
                    RUN curl -L "https://github.com/docker/compose/releases/download/1.25.3/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
                    RUN chmod +x /usr/local/bin/docker-compose
                    '''

                    writeFile file: 'Dockerfile.build', text: dockerfile

                    docker.build('uwc_builder', '-f Dockerfile.build --build-arg http_proxy=http://proxy-chain.intel.com:911 --build-arg https_proxy=http://proxy-chain.intel.com:912 .')
                }
            }
        }

        stage('Build') {
            agent {
                docker {
                    image 'uwc_builder'
                    reuseNode true
                    args '-v /var/run/docker.sock:/var/run/docker.sock'
                }
            }
            // environment { //Dockerfiles do not adhere to the DOCKER_REGISTRY syntax, so this fails on fresh builds
            //     DOCKER_REGISTRY = "uwc_${GIT_BRANCH}_"
            // }
            stages {
                stage('Verify') {
                    steps {
                        // verify docker-compose is installed properly
                        sh 'docker-compose --version'
                    }
                }

                // let's clone the IEdgeInsights project into the workspace
                stage('Clone IEdgeInsights') {
                    steps {
                        dir('IEdgeInsights') {
                            checkout scm: [
                                $class: 'GitSCM',
                                userRemoteConfigs: [
                                    [ url: 'https://gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights.git', credentialsId: 'owr-jenkins-gitlab' ]
                                ],
                                branches: [[name: env.EIS_BRANCH]]
                            ], poll: false
                        }

                        // copy docker-compose
                        sh 'cp docker-compose.yml IEdgeInsights/docker_setup/'

                        // Copy source code into IEdgeInsights directory
                        sh 'cp -r modbus-master IEdgeInsights/'
                        sh 'cp -r MQTT IEdgeInsights/'
                        sh 'cp -r mqtt-export IEdgeInsights/'
                    }
                }

                stage('Prep EIS Base Layers') {
                    steps {
                        // this is temporary for v2.1-Alpha-RC4, it looks to have been fixed in v2.1
                        // sh 'patch IEdgeInsights/common/dockerfiles/Dockerfile.eisbase Release/Dockerfile.eisbase.patch'

                        //base
                        sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy ia_eisbase'
                        // common
                        sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy ia_common'
                    }
                }

                // This stage would only run when not on the master branch...so it skips Klocwork
                stage('Compile') {
                    when { not { branch 'master' } }
                    steps {
                        script {
                            parallel(['Modbus TCP': {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy modbus-tcp-master'
                            }, 'Modbus RTU': {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy modbus-rtu-master'
                            }, 'MQTT Export': {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy mqtt-export'
                            }])
                        }
                    }
                }

                // Klocwork only happens on the master branch
                stage('Klockwork Prep') {
                    when { branch 'master' }
                    environment { 
                        //https://jenkins.io/doc/book/pipeline/syntax/#parameters
                        // this will expose KW_USR and KW_PSW, this will be used to write a file for Klocwork
                        KW = credentials('sys_rrpprotx_kw')
                    }
                    steps {
                        // copy Dockerfiles
                        sh 'cp .kw/modbus-master/* IEdgeInsights/modbus-master/'
                        sh 'cp .kw/mqtt-export/* IEdgeInsights/mqtt-export/'

                        sh 'echo "$KW_USR\n$KW_PSW" > IEdgeInsights/modbus-master/kwcreds'
                        sh 'echo "$KW_USR\n$KW_PSW" > IEdgeInsights/mqtt-export/kwcreds'
                    }
                }

                // Klocwork only happens on the master branch
                stage('Klockwork') {
                    when { branch 'master' }
                    stages {
                        stage('Modbus TCP') {
                            steps {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy --build-arg GIT_BRANCH --build-arg BUILD_NUMBER modbus-tcp-master'
                            }
                        }
                        stage('Modbus RTU') {
                            steps {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy --build-arg GIT_BRANCH --build-arg BUILD_NUMBER modbus-rtu-master'
                            }
                        }
                        stage('MQTT Export') {
                            steps {
                                sh 'cd IEdgeInsights/docker_setup && docker-compose --env-file .env build --build-arg http_proxy --build-arg https_proxy --build-arg GIT_BRANCH --build-arg BUILD_NUMBER mqtt-export'
                            }
                        }
                    }
                }
            }
        }

        stage('RBHE Static Code Scan') {
            steps {
                // remove IEdgeInsights before staticCodeScan to only scan the relavant code
                sh 'rm -rf IEdgeInsights'
                rbheStaticCodeScan()
            }
        }

        // this validation is just an example of how validation can be done on a different node
        // no need to spawn another job, it can just be done inline. If validation is long running,
        // I would suggest only running validation on a cron, not each commit
        /*stage('Validation') {
            when {
                environment name: 'VALIDATION_REQUIRED', value: 'true'
            }
            agent { label 'uwc-validation' }
            steps {
                sh './execute-validation.sh'
            }
        }*/
    }
    post {
        failure {
            slackBuildNotify([failed: true, slackFailureChannel: env.SLACK_FAIL]) {}
        }
        success {
            slackBuildNotify([slackSuccessChannel: env.SLACK_SUCCESS]) {}
        }
    }

}
