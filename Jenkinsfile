pipeline {
    agent { label 'UWC' }
    options {
        timestamps()
    }
    environment {
        // optional, default 'checkmarx,protex', available: protex,checkmarx,sonarqube,klocwork,protecode
        // SCANNERS = 'checkmarx,protex'

        PROJECT_NAME = 'UWC'
        SLACK_SUCCESS = '#indu-uwc'
        SLACK_FAIL   = '#indu-uwc'
    }
    stages {
        stage('Protex Scan') {
            steps {
                staticCodeScan {
                    scanners = ['protex']
                    protexProjectName = 'UWC'
                    protexBuildName = 'rrp-generic-protex-build'
                }
            }
        }
        stage('Code Scan') {
            stages {
                parallel {
                    stage('Static Code Analysis') {
                        steps {
                            staticCodeScan  {
                                scanners = ['sonarqube']
                                scannerType = 'C,C++'
                                projectName = 'UWC' 
                            }
                        }
                    }
                    stage('Static Code Analysis') {
                        steps {
                            staticCodeScan  {
                                scanners = ['checkmarx']
                                checkmarxProjectName = 'UWC' 
                            }
                        }
                    }
                }
            }
        }
    }
}
