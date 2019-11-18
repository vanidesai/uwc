pipeline {
    agent { label 'rbhe' }
    options {
        timestamps()
    }
    environment {
        // optional, default 'checkmarx,protex', available: protex,checkmarx,sonarqube,klocwork,protecode
        SCANNERS = 'sonarqube'

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
        stage('Bandit') {
            agent {
                docker {
                    image 'amr-registry.caas.intel.com/rrp-devops/bandit-build-agent:latest'
                    reuseNode true
                }
            }

            steps {
                sh 'bandit -f txt **/*.py scripts/*.py | tee bandit_scan.txt'
            }
        }
    }
    
    post {
        always {
            archiveArtifacts allowEmptyArchive: true, artifacts: 'bandit_scan.txt'
        }
        failure {
            slackBuildNotify([failed: true, slackFailureChannel: env.SLACK_FAIL]) {}
        }
        success {
            slackBuildNotify([slackSuccessChannel: env.SLACK_SUCCESS]) {}
        }
    }

}
