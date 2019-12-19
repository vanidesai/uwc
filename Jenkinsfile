  
pipeline {
    agent { label 'rbhe' }
    options {
        timestamps()
    }
    environment {
        // optional, default 'checkmarx,protex', available: protex,checkmarx,sonarqube,klocwork,protecode
        SCANNERS = 'checkmarx,protex'
        SCANNERTYPE= 'c,c++'

        PROJECT_NAME = 'UWC'
        SLACK_SUCCESS = '#indu-uwc'
        SLACK_FAIL   = '#indu-uwc'
        GITLAB_UP = credentials('gitlab-username-password')
    }
    stages {
        stage('Hello') {
            steps {
                echo 'Hello..'
                sh "git --version"
                echo "$GITLAB_UP"
                sh "git clone --single-branch --branch v2.1-Alpha-RC4 https://$GITLAB_UP@gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights"
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
