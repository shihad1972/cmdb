#!/usr/bin/perl -w
#
# bb-mysql - mysql check and metrics
# cgoyard:2006-07-18
 
use strict;
 
my $DEBUG       = 0;
 
$ENV{BBPROG}    = "bb-mysql.pl";
my $TESTNAME    = "mysql";
 
my $BBHOME      = $ENV{BBHOME};
my $BB          = $ENV{BB};       # full path to the bin/bb util
my $BBDISP      = $ENV{BBDISP};   # IP of the BBDISPLAY server
my $BBVAR       = $ENV{BBVAR};
my $MACHINE     = $ENV{MACHINE};  # hostname, fqdn
my $COLOR       = "clear";        # global color for the test
my $MSG         = "";             # body of the message
my $HEAD        = "";             # first line of the message (short, optional)
my $DATA        = "";             # data for NCV records (hobbit only)
 
if ($DEBUG == 1) {
        $BBHOME  |= "/tmp";
        $BB           = "/bin/echo";
        $BBDISP  |= "127.0.0.1";
        $BBVAR   |= "/tmp";
        $MACHINE |= "test.host.cvf";
}
 
sub clear; sub green; sub yellow; sub red;
sub setcolor; sub head; sub msg; sub data;
sub sendreport; sub resetreport;
 
 
######################################################################
# here we go
############
 
# First, create a dumb user that can do status and show variables :
# grant select on devnull.* to monitoring@localhost identified by 'monitoring';
# revoke all on devnull.* from monitoring@localhost;
# or use the config file $BBHOME/etc/bb-mysql.cfg to set for example :
# mysqlclient=/some/other/client
# auth=-uthisuser -pthatpassword -Sothersocket
 
 
my $auth    = "-uhobbit -ph0881t";
my $client  = "/usr/bin/mysql";
my $confnotfound = 0;
if(open(CONF, "$BBHOME/etc/bb-mysql.cfg")) {
    my $line;
    while($line = <CONF>) {
        if($line =~ /^mysqlclient(\s+|=)(.+)$/) {
            $client = $2;
        }
        elsif($line =~ /^auth(\s+|=)(.+)/) {
            $auth = $2;
        }
    }
    close(CONF);
}
else {
    $confnotfound = 1;
}
 
my (@output, $version, $metrics, $uptime, $maxcon, $ts);
@output = `$client $auth -Bs -e "select 'DATE', now() ; status; show variables"`;
if($? == 0 and $output[0] =~ /^DATE\s+(.+)$/) {
    $ts = $1;
    green;
}
else {
    head("MySQL Server DOWN");
    msg("&red MySQL Server is broken");
    msg("(configuration file was not found") if $confnotfound;
    red;
    sendreport;
    exit 1;
}
 
# metrics
foreach (@output) {
    if ( /^Server version:\s+(.+)$/ )    { $version = $1     }
    elsif ( /^Uptime:\s+(.+)/ )          { $uptime  = $1     }
    elsif ( /^(Threads:.+)$/ )           { $metrics = lc($1) }
    elsif ( /^max_connections\s+(\d+)/ ) { $maxcon  = $1     }
}
 
$metrics =~ s/(\d)\s+/$1\n/g; # remplace les espaces qui suivent un chiffre par des \n
msg $metrics;
head("MySQL Server OK");
msg "maxcon: $maxcon\n";
msg "Server version: $version\nUptime: $uptime\n";
msg("Server reports timestamp of $ts");
sendreport;
exit 0;
 
 
 
 
######################################################################
# toolbox
###########
 
####
# sends the report
####
 
sub sendreport
{
    $MACHINE =~ s/\./,/g;
    my $date = localtime;
    my $cmd = "$BB $BBDISP \"status $MACHINE.$TESTNAME $COLOR $date $HEAD\n$DATA\n$MSG\"";
    system($cmd);
}
 
sub resetreport
{
    $MSG = $DATA = $HEAD = '';
    $COLOR = 'clear';
}
 
# sets the global color of the test
# prevents downgrading severity
# clear == green < yellow < red
sub setcolor
{
    my $newcolor = shift;
    if($newcolor eq "red") {
        $COLOR = "red";
    }
    elsif($COLOR eq "green" or $COLOR eq "clear") {
        $COLOR = "$newcolor";
    }
 
    return $COLOR;
}
sub clear  { setcolor 'clear'  }
sub green  { setcolor 'green'  }
sub yellow { setcolor 'yellow' }
sub red    { setcolor 'red'    }
 
 
sub data
{
    my ($n, $v) = @_;
    $DATA .= "$n: $v\n";
}
 
sub head
{
    $HEAD = "@_";
}
 
sub msg
{
    $MSG .= join("\n", @_) . "\n";
}

