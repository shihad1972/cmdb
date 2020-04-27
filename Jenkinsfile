pipeline {
    agent { docker { image 'php' } }
    stages {
        stage('build') {
            steps {
                sh 'make --version'
            }
        }
    }
}

