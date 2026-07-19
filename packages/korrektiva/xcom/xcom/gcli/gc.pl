#!/usr/bin/perl

select STDERR; $| = 1;      # make unbuffered
select STDOUT; $| = 1;      # make unbuffered

my $Version="2005-05-19";
print "X-Com client (gc), v. $Version\n";

use Socket;
use Carp;
use Win32::Registry;
use File::Glob;

# Detect platform 
#
my ($os,$osl,$ht,$htl,$platf);

if ($ENV{OS} =~ /windows/i) { 
  $os = 'Windows'; $osl = 'W'; 
  $ht = 'Intel'; $htl='I'; 
}else{ 
  my $una0 = `uname -a`;
  if ($una0 =~ /linux/i) { $os = 'Linux'; $osl = 'L'; }
  if ($una0 =~ /i\d86/i) { $ht = 'Intel'; $htl = 'I'; }
  elsif ($una0 =~ /x86/i) { $ht = 'x86'; $htl = 'X'; } # x86_64
  elsif ($una0 =~ /alpha/i) { $ht = 'Alpha'; $htl = 'A'; }
}
die "Error: unknown operation system!\n" if (!defined $osl);
die "Error: unknown hardware!\n" if (!defined $htl);


$platf = $osl.$htl;

print "Operating system: <$os> (letter='$osl')\n";
print "Hardware: <$ht> (letter='$htl')\n";
print "Platform code: <$platf>\n";


# Get access to tar & gzip for windows
#
if ($os eq 'Windows') {
  my $dir = `cd`;
  chomp $dir;
  $ENV{PATH}=".;$dir;$ENV{PATH}";
}


# Sleep settings
#
my $sleep1 = 5;
my $sleep2 = 5;


# Host and port to connect
#
my ($host, $port);
if ($ARGV[0] =~ /^(.*)?:(\d+)$/) { $host = $1; $port = $2; } else { $host = $ARGV[0]; $port = 80; }
die ("Error: server host name must be specified!") if (!$host);
print "X-Com server: host=<$host>, port=<$port>\n";


# Get host name
#
my $una = '';
if ($os eq 'Windows') 
{
  $una=`hostname`;
  chomp $una;
  if ($una eq '')
  {
    my $ipcfg = `ipconfig`;
    if ($ipcfg =~ /ip.*?(\d+\.\d+\.\d+\.\d+)/i) { $una = $1; }
  }
}
else
{ 
  $una = (split /\s/,`uname -a`)[1];
}
if ($una eq '') { print STDERR "Warning: can't get host name.\n"; }
print "Host name is: <$una>\n";


# Get processor frequency
#
my $mhz = 0;
if ($os eq 'Windows') {
  my $tips;
  $::HKEY_LOCAL_MACHINE->Open("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", $tips);#	or die "Can't open processor: $^E";
  my ($type, $value);
  $tips->QueryValueEx("~MHz", $type, $value);# or die "No ~MHz: $^E";
  if ($value) { $mhz = $value; }
}else{
  my $cpuinfo = `cat /proc/cpuinfo`;
  if    ($cpuinfo =~ /timer frequency.*:\s*([\d\.]+)/i)	{ $mhz = $1; }
  elsif ($cpuinfo =~ /cpu\s*mhz.*:\s*([\d\.]+)/i) 	{ $mhz = $1; }
  else  { print STDERR "Warning: can't detect processor frequency.\n"; } 
}
$mhz = 0 + $mhz;
print "Processor frequency: $mhz\n";


# Read and process node id and tmp file
#
my $id = $ARGV[1];
my $tmpid = $ARGV[2];
my $mpiid = $ENV{GMPI_ID};
my $rnd = sprintf "%05d-%05d",time%100000,rand(100000);
if (($id eq '0')||(!defined($id))) { $id = "UNA.RND"; print STDERR "Warning: node ID is not defined. Use <$id>.\n"; }
if (($tmpid eq '0')||(!defined($tmpid))) { $tmpid = "UNA.RND"; print STDERR "Warning: temporary name is not defined. Use <$tmpid>.\n"; }
$id	=~ s/UNA/$una/g;
$id	=~ s/RND/$rnd/g;
$id	=~ s/MPI_ID/$mpiid/g;
$tmpid	=~ s/UNA/$una/g;
$tmpid	=~ s/RND/$rnd/g;
$tmpid	=~ s/MPI_ID/$mpiid/g;
print "Node id: <$id>\n";
print "Temporary id: <$tmpid>\n";


# Set home directory and HTTP prefix
#
if ((defined $ARGV[3])&&($ARGV[3] ne '0')) 
{ 
  $ENV{HOME} = $ARGV[3]; 
}
else
{
  if ($os eq 'Windows') 
  {
    if (defined $ENV{HOMEPATH} && defined $ENV{HOMEDRIVE}) { $ENV{HOME}=$ENV{HOMEDRIVE}.$ENV{HOMEPATH}; }    
    else { if (defined $ENV{USERPROFILE}) { $ENV{HOME}=$ENV{USERPROFILE}; } }
  }
}
print "Home directory: <$ENV{HOME}>\n";
my $http_prefix = $ARGV[4];
print "HTTP prefix: <$http_prefix>\n" if ($http_prefix);
print "\n";

my $task;
my $taskarg;
my $portion;
my $session;
my $worktime;
my $srclen;


# Ignore broken pipes
#
$SIG{PIPE} = 'IGNORE';


# Subroutine: Open a TCP connection
#
sub open_TCP
{
  # get parameters
  my ($FS, $dest, $port) = @_;
 
  my $proto = getprotobyname('tcp');
  socket($FS, PF_INET, SOCK_STREAM, $proto);
  my $sin = sockaddr_in($port,inet_aton($dest));
  if (!socket($FS,PF_INET,SOCK_STREAM,$proto)) { print STDERR "Socket error: $!\n"; return undef; } 
  if (!connect($FS, $sin)) { print STDERR "Connect error: $!\n"; return undef; };
  
  my $old_fh = select($FS); 
  $| = 1; 		        # don't buffer output
  select($old_fh);
  1;
}

sub http_get
{
  my ($server,$port,$url) = @_;

  open_TCP F, $server, $port;
  open FI, ">$tmpid-in.dat";
  binmode F;
  binmode FI;

  print F "GET $url HTTP/1.0\n";
  print F "\n";

  $portion = '';
  $session = '';
  $task = '';
  $taskarg = '';
  while(<F>) { chop;chop;last if ($_ eq '');}
  my $iline = 0;
  while(<F>) {
    chomp;
    $line = $_;
    if ($iline == 0) { $session = $line; }
    elsif ($iline == 1) { $task = $line; }
    elsif ($iline == 2) { $taskarg = $line; }
    elsif ($iline == 3) { 
	$portion = $line; 
	my $t1=$/; my $t2=$\; undef $\; undef $/;
	print FI <F>; 
	$/ = $t1; $\ = $t2; 
	last;
    }
    $iline++;
  }

  close(F);
  close(FI);
  if (-z "$tmpid-in.dat") { unlink <$tmpid-in.dat>; return 0; }
  else { return 1; }
}

sub http_get_src
{
  my ($server,$port,$url) = @_;

  open_TCP FS, $server, $port;
  open FI, ">$tmpid-in.dat";
  binmode FS;
  binmode FI;

  print FS "GET $url HTTP/1.0\n";
  print FS "\n";

#  print STDERR "GET $url HTTP/1.0\n";

  $session = '';
  $task = '';
  $srclen = '';
  while(<FS>) { chop;chop;last if ($_ eq '');}
  my $iline = 0;
  while(<FS>) {
    chomp;
    $line = $_;
    if ($iline == 0) { $session = $line; print "session=$session\n";}
    if ($iline == 1) { $task = $line; print "task=$task\n"; }
    elsif ($iline == 2) { 
	$srclen = $line;
	print "srclen=$srclen\n"; 
        my $t1=$/; my $t2=$\; undef $\; undef $/;
        print FI <FS>;
        $/ = $t1; $\ = $t2;
        last;
    }
    $iline++;
  }

  close(FS);
  close(FI);
  if (-z "$tmpid-in.dat") { unlink <$tmpid-in.dat>; return 0; }
  else { return 1; }
}

sub http_post 
{
  my ($server,$port,$url) = @_;

  open_TCP FF, $server, $port;
  open FI, ">$tmpid-in.dat";
  open FO, "<$tmpid-out.dat";
  binmode FF;
  binmode FI;
  binmode FO;

  print FF "POST $url HTTP/1.0\n";

  my $t1=$/; my $t2=$\; undef $/; undef $\;
  my $buf = <FO>;
  print FF "Content-Type: application/octet-stream\n";
  print FF "Content-length: ".length($buf)."\n\n$buf";
# print stderr "Content-length: ".length($buf)."\n";
  $/ = $t1; $\ = $t2; 
  $portion = '';
  $session = '';
  $task = '';
  while(<FF>) { chop;chop;last if ($_ eq ''); }

  my $iline = 0;
  while(<FF>) {  
    chomp;
    $line = $_;
    if ($iline == 0) { $session = $line; }
    elsif ($iline == 1) { $task = $line; }
    elsif ($iline == 2) { $taskarg = $line; } 
    elsif ($iline == 3) { 
	$portion = $line; 
	my $t1=$/; my $t2=$\; undef $/; undef $\;
	print FI <FF>; 
	$/ = $t1; $\ = $t2; 
	last;
    }
    $iline++; 
  }

  close(FF);
  close(FI);
  close(FO);

  if (-z "$tmpid-in.dat") { unlink <$tmpid-in.dat>; return 0; }
  else { return 1; }
}


xstart:

my $maintask = '';
while(1) {
  if (! -d "$ENV{HOME}/$tmpid")
  {
    print "\nCreating temporary directory for the task: $ENV{HOME}/$tmpid\n";
    mkdir "$ENV{HOME}/$tmpid", 0777;
  }
  chdir "$ENV{HOME}/$tmpid";

  print "Trying to get a task source from the server...\n";
  http_get_src $host,$port,"$http_prefix/?sId=$session&gcCode=$id&opCode=4&OS=$platf&MHz=$mhz";
  if (-e "$tmpid-in.dat") {
    $maintask = $task;
    print "Creating directory: $ENV{HOME}/$tmpid/$task\n";
    mkdir "$ENV{HOME}/$tmpid/$task", 0777;
    rename "$tmpid-in.dat", "$task/$tmpid.tar.gz";
    chdir "$ENV{HOME}/$tmpid/$task";
    print "Task source received.\n";
    if ((-s "$tmpid.tar.gz") == $srclen) {
      print "Unpacking source...\n";
      `gzip -d $tmpid.tar.gz`;
      `tar xvf $tmpid.tar`;
      unlink <$tmpid.tar*>;
      print "Source ready. Running prepare...\n";
      my $ret = eval { do "gctask"; gcprepare(); };
      if ($ret == 1) { print "Source test passed.\n"; last; }
      else           { print "Source test not passed!\n"; } 
    }else{
      print STDERR "Error: incorrect source size! (",(-s "tmpid.tar.gz"),")\n";
    }
  } 
  unlink <$tmpid*>;
  my $sleep = int($sleep1 + rand($sleep2));
  print "No tasks to perform or server is not ready. Sleeping for $sleep seconds...\n\n";
  sleep $sleep;
}



unlink <$tmpid*.dat>;
my $errcnt = 0;
while(1) {

  unlink <$tmpid-in.dat>;
  if ((-e "$tmpid-out.dat") && $portion) {
    print "Posting $portion\n";
    http_post $host,$port,"$http_prefix/?sId=$session&opCode=2&gcCode=$id&OS=$platf&MHz=$mhz&portion=$portion&task=$task&time=$worktime";
  }else{
    print "Requesting data for the first time\n";
    http_get $host,$port,"$http_prefix/xcom/?sId=$session&opCode=1&gcCode=$id&OS=$platf&MHz=$mhz";
  }

  if (-e "$tmpid-in.dat") {
    print "Doing portion $portion...\n";
    unlink <$tmpid-out.dat>; 

    if ($task != $maintask) { goto xstart; }
    my $t1 = time;
    my $ret = eval {gctask($task,$taskarg,$portion,"$tmpid-in.dat","$tmpid-out.dat");};
    $worktime = time-$t1;

    if ((-e "$tmpid-out.dat") && ($ret == 1)) { 
	$errcnt = 0; 
	print "Done\n"; 
        next;
    } 
  }

  goto xstart;
#  if ($errcnt++ >= 10) { print "Maximum connection errors occured. Full restart.\n"; goto xstart; }
#  unlink <$tmpid-out.dat>;
#  my $sleep = int($sleep1 + rand($sleep2));
#  print "Get a connection error ($errcnt). Sleeping for $sleep seconds...\n";
#  sleep $sleep;
}
