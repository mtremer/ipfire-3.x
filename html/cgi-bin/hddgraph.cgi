#!/usr/bin/perl
#
# SmoothWall CGIs
#
# This code is distributed under the terms of the GPL
#
# (c) The SmoothWall Team
#

use strict;

# enable only the following on debugging purpose
# use warnings;
# use CGI::Carp 'fatalsToBrowser';

require '/var/ipfire/general-functions.pl';
require "${General::swroot}/lang.pl";
require "${General::swroot}/header.pl";

my %cgiparams=();
my @cgigraphs=();
my @graphs=();

&Header::showhttpheaders();

my $graphdir = "/home/httpd/html/graphs";

&Header::getcgihash(\%cgiparams);

$ENV{'QUERY_STRING'} =~ s/&//g;
@cgigraphs = split(/graph=/,$ENV{'QUERY_STRING'});
$cgigraphs[1] = '' unless defined $cgigraphs[1];

my %mbmon_settings = ();
my %mbmon_values = ();
&General::readhash("/var/log/mbmon-values", \%mbmon_values);
my $key;

if ( $cgiparams{'ACTION'} eq $Lang::tr{'save'} )
{
  $mbmon_settings{'GRAPH_TEMP'} = ($cgiparams{'TEMP'} eq 'on');
  $mbmon_settings{'GRAPH_FAN'} = ($cgiparams{'FAN'} eq 'on');
  $mbmon_settings{'GRAPH_VOLT'} = ($cgiparams{'VOLT'} eq 'on');
  $mbmon_settings{'GRAPH_HDD'} = ($cgiparams{'HDD'} eq 'on');

  foreach my $line (sort keys %cgiparams) 
  {
    if ( index($line, "LINE-") != -1 )
    {
      $mbmon_settings{$line} = 'on';
    }

    if ( index($line, "LABEL-") != -1 )
    {
      $mbmon_settings{$line} = $cgiparams{$line};
    }
  }

  &General::writehash("${General::swroot}/mbmon/settings", \%mbmon_settings);
}
else
{
  &General::readhash("${General::swroot}/mbmon/settings", \%mbmon_settings);
}

my $selected_temp = '';
my $selected_fan = '';
my $selected_volt = '';
my $selected_hdd = '';

$selected_temp = "checked='checked'" if ( $mbmon_settings{'GRAPH_TEMP'} == 1 );
$selected_fan  = "checked='checked'" if ( $mbmon_settings{'GRAPH_FAN'} == 1 );
$selected_volt = "checked='checked'" if ( $mbmon_settings{'GRAPH_VOLT'} == 1 );
$selected_hdd = "checked='checked'" if ( $mbmon_settings{'GRAPH_HDD'} == 1 );

my %mbmon_graphs = ();
foreach $key ( sort(keys %mbmon_values) ) 
{
  $mbmon_graphs{$key} = "checked='checked'" if ( $mbmon_settings{'LINE-'.$key} eq 'on' );
  if ( !defined($mbmon_settings{'LABEL-'.$key}) )
  {
    $mbmon_settings{'LABEL-'.$key} = $key;
  }
}

&Header::openpage($Lang::tr{'harddisk temperature graphs'}, 1, '');

&Header::openbigbox('100%', 'left');

###############
# DEBUG DEBUG
#&Header::openbox('100%', 'left', 'DEBUG');
#my $debugCount = 0;
#foreach my $line (sort keys %cgiparams) {
#  print "$line = $cgiparams{$line}<br />\n";
#  $debugCount++;
#}
#print "&nbsp;Count: $debugCount\n";
#&Header::closebox();
# DEBUG DEBUG
###############

if ($cgigraphs[1] =~ /(temp|fan|volt)/) 
{
  my $graph = $cgigraphs[1];
  my $graphname = $Lang::tr{"mbmon $cgigraphs[1]"};
  &Header::openbox('100%', 'center', "$graphname $Lang::tr{'graph'}");

  if (-e "$graphdir/mbmon-${graph}-day.png") 
  {
    my $ftime = localtime((stat("$graphdir/mbmon-${graph}-day.png"))[9]);
    print "<center>";
    print "<b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br /><hr />\n";
    print "<img src='/graphs/mbmon-${graph}-day.png' border='0' /><hr />";
    print "<img src='/graphs/mbmon-${graph}-week.png' border='0' /><hr />";
    print "<img src='/graphs/mbmon-${graph}-month.png' border='0' /><hr />";
    print "<img src='/graphs/mbmon-${graph}-year.png' border='0' />";
  }
  else 
  {
    print $Lang::tr{'no information available'};
  }
  &Header::closebox();
  print "<div align='center'><table width='80%'><tr><td align='center'>";
  print "<a href='/cgi-bin/mbmongraph.cgi'>";
  print "$Lang::tr{'back'}</a></td></tr></table></div>\n";
}
elsif ($cgigraphs[1] =~ /(hdd)/) 
{
  my $graph = $cgigraphs[1];
  my $graphname = $Lang::tr{"harddisk temperature"};
  &Header::openbox('100%', 'center', "$graphname $Lang::tr{'graph'}");

    if (-e "$graphdir/hddtemp-day.png")
     {
      my $ftime = localtime((stat("$graphdir/hddtemp-day.png"))[9]);
      print "<center>";
      print "<b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br /><hr />\n";
      print "<img src='/graphs/hddtemp-day.png' border='0' /><hr />";
      print "<img src='/graphs/hddtemp-week.png' border='0' /><hr />";
      print "<img src='/graphs/hddtemp-month.png' border='0' /><hr />";
      print "<img src='/graphs/hddtemp-year.png' border='0' />";
      if ( -e "/var/log/hddgraph_smartctl_out" ) 
      {
        my $output = `/bin/cat /var/log/hddgraph_smartctl_out`;
        $output = &Header::cleanhtml($output);
        print "<hr><pre>$output</pre>\n";
      }
    }
    else 
    {
      print $Lang::tr{'no information available'};
    }
  &Header::closebox();
  print "<div align='center'><table width='80%'><tr><td align='center'>";
  print "<a href='/cgi-bin/mbmongraph.cgi'>";
  print "$Lang::tr{'back'}</a></td></tr></table></div>\n";
}
else 
{
  if ( $mbmon_settings{'GRAPH_TEMP'} == 1 )
  {
    &Header::openbox('100%', 'center', "$Lang::tr{'mbmon temp'} $Lang::tr{'graph'}");
    if (-e "$graphdir/mbmon-temp-day.png") 
    {
      my $ftime = localtime((stat("$graphdir/mbmon-temp-day.png"))[9]);
      print "<center><b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br />\n";
      print "<a href='/cgi-bin/mbmongraph.cgi?graph=temp'>";
      print "<img src='/graphs/mbmon-temp-day.png' border='0' />";
      print "</a>";
    }
    else 
    {
      print $Lang::tr{'no information available'};
    }
    print "<br />\n";
    &Header::closebox();
  }

  if ( $mbmon_settings{'GRAPH_FAN'} == 1 )
  {
    &Header::openbox('100%', 'center', "$Lang::tr{'mbmon fan'} $Lang::tr{'graph'}");
    if (-e "$graphdir/mbmon-fan-day.png") 
    {
      my $ftime = localtime((stat("$graphdir/mbmon-fan-day.png"))[9]);
      print "<center><b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br />\n";
      print "<a href='/cgi-bin/mbmongraph.cgi?graph=fan'>";
      print "<img src='/graphs/mbmon-fan-day.png' border='0' />";
      print "</a>";
    }
    else 
    {
      print $Lang::tr{'no information available'};
    }
    print "<br />\n";
    &Header::closebox();
  }

  if ( $mbmon_settings{'GRAPH_VOLT'} == 1 )
  {
    &Header::openbox('100%', 'center', "$Lang::tr{'mbmon volt'} $Lang::tr{'graph'}");
    if (-e "$graphdir/mbmon-volt-day.png") 
    {
      my $ftime = localtime((stat("$graphdir/mbmon-volt-day.png"))[9]);
      print "<center><b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br />\n";
      print "<a href='/cgi-bin/mbmongraph.cgi?graph=volt'>";
      print "<img src='/graphs/mbmon-volt-day.png' border='0' />";
      print "</a>";
    } 
    else 
    {
      print $Lang::tr{'no information available'};
    }
    print "<br />\n";
    &Header::closebox();
  }

  if ( $mbmon_settings{'GRAPH_HDD'} == 1 )
  {
    &Header::openbox('100%', 'center', $Lang::tr{'harddisk temperature'});
    if (-e "$graphdir/hddtemp-day.png")
     {
      my $ftime = localtime((stat("$graphdir/hddtemp-day.png"))[9]);
      print "<center>";
      print "<b>$Lang::tr{'the statistics were last updated at'}: $ftime</b></center><br /><hr />\n";
      print "<a href='/cgi-bin/hddgraph.cgi?graph=hdd'>";
      print "<img src='/graphs/hddtemp-day.png' border='0' /><hr />";
      print "</a>";
    }
    else 
    {
      print $Lang::tr{'no information available'};
    }
    print "<br />\n";
    &Header::closebox();
  }

  &Header::openbox('100%', 'center', $Lang::tr{'settings'});
print <<END
<form method='post' action='$ENV{'SCRIPT_NAME'}'>
<table width='100%'>
<tr><td colspan='2'><input type='checkbox' name='TEMP' $selected_temp />&nbsp;$Lang::tr{'mbmon temp'} $Lang::tr{'graph'}</td></tr>
<tr><td colspan='2'><input type='checkbox' name='FAN' $selected_fan />&nbsp;$Lang::tr{'mbmon fan'} $Lang::tr{'graph'}</td></tr>
<tr><td colspan='2'><input type='checkbox' name='VOLT' $selected_volt />&nbsp;$Lang::tr{'mbmon volt'} $Lang::tr{'graph'}</td></tr>
<tr><td colspan='2'><input type='checkbox' name='HDD' $selected_hdd />&nbsp;$Lang::tr{'harddisk temperature'}-$Lang::tr{'graph'}</td></tr>
</table>
<hr />
<table width='100%' border='0' cellspacing='1' cellpadding='0'>
<tr><td align='center' width='10%'><b>$Lang::tr{'mbmon display'}</b></td><td align='center' width='15%'>&nbsp;</td><td align='center' width='15%'><b>$Lang::tr{'mbmon value'}</b></td><td align='left'><b>$Lang::tr{'mbmon label'}</b></td></tr>
END
;

my $i = 0;
foreach $key ( sort(keys %mbmon_values) ) 
{
  if ( $i % 2 )
  {
    print("<tr bgcolor='$Header::table2colour'>");
  }
  else 
  {
    print("<tr bgcolor='$Header::table1colour'>");
  }
  $mbmon_settings{'LABEL-'.$key} = &Header::cleanhtml($mbmon_settings{'LABEL-'.$key});
  print("<td align='center'><input type='checkbox' name='LINE-$key' $mbmon_graphs{$key}/></td>");
  print("<td>$key</td><td align='center'>$mbmon_values{$key}</td>\n");
  print("<td>&nbsp;<input type='text' name='LABEL-$key' value='$mbmon_settings{'LABEL-'.$key}' size='25' /></td></tr>\n");
  $i++;
}

print <<END
</table>

<table width='100%'>
<tr><td class='base' valign='top'>&nbsp;</td><td width='40%' align='center'><input type='submit' name='ACTION' value='$Lang::tr{'save'}' /></td></tr>
</table>

</form>
END
;
  &Header::closebox();
}

&Header::closebox();

&Header::closebigbox();
&Header::closepage();
