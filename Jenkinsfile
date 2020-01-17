  
pipeline {
    //agent { label 'rbhe' }
    agent {
        docker { label 'rbhe'
		image 'ubuntu:18.04' }
    }
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
    }
    stages {
       
        stage('Static Scanners') {
	    //when { branch "UWC-Sprint5" }
            steps {
                echo 'Protex, checkmarx Scan..'
                rbheStaticCodeScan()
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
