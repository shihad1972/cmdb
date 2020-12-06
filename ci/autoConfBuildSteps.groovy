def autoInit() {
        sh "git log --stat --name-only --date=short --abbrev-commit > ChangeLog"
        sh "autoreconf -iv"
}

def clean() {
        sh "./configure --sysconfdir=/etc --localstatedir=/var/lib"
        sh "make distclean"
}

def check() {
        sh "./configure --sysconfdir=/etc --localstatedir=/var/lib"
        sh "make distcheck"
}

def install() {
        sh "./configure --sysconfdir=/etc --localstatedir=/var/lib"
        sh "make"
        sh "sudo make install"
}
