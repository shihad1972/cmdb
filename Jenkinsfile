pipeline {
    agent {
        docker {
            alwaysPull true
            image 'cmdb'
            registryUrl "https://docker.shihad.org:5000"
        }
    }
    stages {
        stage('distcheck') {
            steps {
                def autoconf = load 'ci/autoConfBuildSteps.groovy'
                autoconf.init
                autoconf.clean
                autoconf.check
            }
        }
        stage('install') {
            steps {
                def autoconf = load 'ci/autoConfBuildSteps.groovy'
                autoconf.install
            }
        }
    }
}
