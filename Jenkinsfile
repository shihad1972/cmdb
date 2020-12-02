pipeline {
    agent {
        docker {
            alwaysPull true
            image 'cmdb'
            registryUrl "https://docker.shihad.org:5000"
        }
    }
    stages {
        stage('build') {
            steps {
                sh '''
                   git log --stat --name-only --date=short --abbrev-commit > ChangeLog
                   autoreconf -iv
                   ./configure --sysconfdir=/etc --localstatedir=/var/lib
                   make distclean
                   ./configure --sysconfdir=/etc --localstatedir=/var/lib
                   make distcheck
                '''
            }
        }
    }
}
