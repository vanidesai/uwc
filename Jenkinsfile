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
        stage('Hello') {
            steps {
                echo 'Hello..'
            }
        }
        /*stage('Protex Scan') {
            steps {
                echo 'Protex Scan..'
                rbheStaticCodeScan()
            }
        }*/
    }
    
    post {
        always {
            archiveArtifacts allowEmptyArchive: true, artifacts: 'bandit_scan.txt'
        }
        failure {
            updateGithubCommitStatus name: 'Static Code Analysis', state: 'failed'
            slackBuildNotify([failed: true, slackFailureChannel: env.SLACK_FAIL]) {}
        }
        success {
            updateGithubCommitStatus name: 'Static Code Analysis', state: 'success'
            slackBuildNotify([slackSuccessChannel: env.SLACK_SUCCESS]) {}
        }
    }

}
