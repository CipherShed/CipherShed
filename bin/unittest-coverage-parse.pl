#!/usr/bin/perl -w

my $pathprefix="src/unit-tests";

my %data;

my $state=0;
my $curfile='';
while (<>)
{
  if (m/^File '(.*)'/)
  {
    my $path=$1;
    $state=1;
    if ($path =~ m|^/|)
    {
      #abs file, must be a lib include
    }
    else
    {

      # fix the paths to project based...
      $path =~ s|^../../|| or $path =~ s|^../|src/| or $path =~ s|^|src/unit-tests/|;

      $path =~ m|^(?:.*/)?([^/]+)$|;
      my $file=$1;

      if ($path=~m|^src/| and not $path=~m|^src/unit-tests/|)
      {
        $state=2;
        $curfile=$file;
	$data{$curfile}{'paths'}{$path}=$path;
      }
    }
  }
  elsif ($state==2 and m/Lines executed:([0-9]+\.[0-9]+)% of (-?[0-9]+)/)
  {
    my $pct=$1;
    my $linecount=$2;
    
    $state=3;
    my $lines=int($pct * $linecount / 100 + 0.5);

    $data{$curfile}{'linecount'}=$linecount;
    $data{$curfile}{'lines'}=$lines;
  }
}

for my $key (sort keys %data)
{
  print "$key: $data{$key}{'lines'} of $data{$key}{'linecount'}\n";
}

