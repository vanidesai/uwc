  
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
        stage('Build') {
		  //when { branch "UWC-Sprint5" }
		  steps {
			  echo 'Hello..'
			  sh "apt update"
			  sh "apt install git -y"
			  sh "git --version"
			  script {
				  withCredentials([
					usernamePassword(credentialsId: 'GITLAB_UP',
					  usernameVariable: 'username',
					  passwordVariable: 'password')
				  ]) {
					print 'username=' + username + 'password=' + password
					upass = username+":"+password
					sh "set +x; git clone --single-branch --branch v2.1-Alpha-RC4 https://"+upass+"@gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights"
					
				  }
				}
			  sh "cp -r ./Release/* ./IEdgeInsights/"
			  sh "cd ./IEdgeInsights/; ls -la"
			  sh "echo \"http_proxy=http://proxy-chain.intel.com:911\" >> /etc/environment"
			  sh "echo \"https_proxy=http://proxy-chain.intel.com:912\" >> /etc/environment"
			  sh "echo \"HTTP_PROXY=http://proxy-chain.intel.com:911\" >> /etc/environment"
			  sh "echo \"HTTPS_PROXY=http://proxy-chain.intel.com:912\" >> /etc/environment"
			  //sh "source /etc/environment"

			  sh "apt-get -y install systemd"
			  sh "cd ./IEdgeInsights/; chmod 777 ./01_pre-requisites.sh; ./01_pre-requisites.sh --proxy proxy-us.intel.com:911; "
			  sh "cd ./IEdgeInsights/; chmod 777 ./02_provisionEIS.sh; ./02_provisionEIS.sh; "
			  sh "cd ./IEdgeInsights/; chmod 777 ./03_DeployEIS.sh; ./03_DeployEIS.sh; "
			  
		  }
          
        }
	stage('KW-Scan') {
		  //when { branch "UWC-Sprint5" }
		  steps {
			  echo 'Hello..'
			  sh "apt update"
			  sh "apt install git -y"
			  sh "git --version"
			  script {
				  withCredentials([
					usernamePassword(credentialsId: 'GITLAB_UP',
					  usernameVariable: 'username',
					  passwordVariable: 'password')
				  ]) {
					print 'username=' + username + 'password=' + password
					upass = username+":"+password
					sh "set +x; git clone --single-branch --branch v2.1-Alpha-RC4 https://"+upass+"@gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights"
					
				  }
				}
			  sh "cp -r ./Release/* ./IEdgeInsights/"
			  sh "cd ./IEdgeInsights/; ls -la"
			  sh "echo \"http_proxy=http://proxy-chain.intel.com:911\" >> /etc/environment"
			  sh "echo \"https_proxy=http://proxy-chain.intel.com:912\" >> /etc/environment"
			  sh "echo \"HTTP_PROXY=http://proxy-chain.intel.com:911\" >> /etc/environment"
			  sh "echo \"HTTPS_PROXY=http://proxy-chain.intel.com:912\" >> /etc/environment"
			  //sh "source /etc/environment"
			  sh "cp ./.kw/modbus-master/*  ./IEdgeInsights/modbus-master/;"
			  sh "cp ./.kw/mqtt-export/*  ./IEdgeInsights/mqtt-export/;"
			  
			  sh "apt-get -y install systemd"
			  sh "cd ./IEdgeInsights/; chmod 777 ./01_pre-requisites.sh; ./01_pre-requisites.sh --proxy proxy-us.intel.com:911; "
			  sh "cd ./IEdgeInsights/; chmod 777 ./02_provisionEIS.sh; ./02_provisionEIS.sh; "
			  sh "cd ./IEdgeInsights/; chmod 777 ./03_DeployEIS.sh; ./03_DeployEIS.sh; "
			  
		  }
          
        }
        //stage('Static Scanners') {
        //    steps {
        //        echo 'Protex, checkmarx Scan..'
        //        rbheStaticCodeScan()
        //    }
        //}
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
