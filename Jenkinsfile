pipeline {
    agent { label 'UWC' }
    options {
        timestamps()
    }
    environment {
        // optional, default 'checkmarx,protex', available: protex,checkmarx,sonarqube,klocwork,protecode
        // SCANNERS = 'checkmarx,protex'

        PROJECT_NAME = 'UWC'
        SLACK_SUCCESS = '#indu-base-algos'
        SLACK_FAIL   = '#indu-base-algos'
    }
    triggers {
        gitlab(triggerOnPush: true, triggerOnMergeRequest: true, branchFilterType: 'All')
    }
    stages {
        stage('Code Scan') {
            parallel {
                stage('Bandit') {
                    agent {
                        docker {
                            image 'amr-registry.caas.intel.com/rrp-devops/bandit-build-agent:latest'
                            reuseNode true
                        }
                    }

                    steps {
                        sh 'bandit -f txt **/*.py scripts/*.py | tee bandit_scan.txt'
                        updateGitlabCommitStatus name: 'Bandit', state: 'success'
                    }
                }
                stage('Static Code Analysis') {
                    steps {
                        rbheStaticCodeScan()
                        updateGitlabCommitStatus name: 'Static Code Analysis', state: 'success'
                    }
                }
            }
        }
    }
    post {
        always {
            archiveArtifacts allowEmptyArchive: true, artifacts: 'bandit_scan.txt'
        }
        failure {
            updateGitlabCommitStatus name: 'Static Code Analysis', state: 'failed'
            slackBuildNotify([failed: true, slackFailureChannel: env.SLACK_FAIL]) {}
        }
        success {
            updateGitlabCommitStatus name: 'Static Code Analysis', state: 'success'
            slackBuildNotify([slackSuccessChannel: env.SLACK_SUCCESS]) {}
        }
    }
}
