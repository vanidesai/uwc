pipeline {
    agent { label 'rbhe' }
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
        stage('Stage 1') {
            steps {
                echo 'Hello world!' 
            }
        }
    }
}
