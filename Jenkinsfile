  
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
    }
    stages {
        stage('Hello') {
		  when { branch "UWC-Sprint3" }
		  steps {
			  echo "$GIT_BRANCH"
			  echo 'Hello..'
			  sh "git --version"
			  echo "$GITLAB_UP"
			  sh "git clone --single-branch --branch v2.1-Alpha-RC4 https://$GITLAB_UP@gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights"
			  sh "cp -r ./Deploy/* ./IEdgeInsights/"
			  sh "cd ./IEdgeInsights/; ls -la"
			  sh "apk add ncurses"
			  sh "pwd"
			  #sh "cd ./IEdgeInsights/; chmod 777 ./01_pre-requisites.sh; ./01_pre-requisites.sh; "

		  }
          
        }
        stage('Static Scanners') {
            steps {
                echo 'Protex, checkmarx Scan..'
                rbheStaticCodeScan()
            }
        }
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
