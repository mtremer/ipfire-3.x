#!/usr/bin/perl
#
# IPFire CGIs
#
# This code is distributed under the terms of the GPL
#
# (c) The IPFire Team

use strict;
# enable only the following on debugging purpose
use warnings;
use CGI::Carp 'fatalsToBrowser';

require '/var/ipfire/general-functions.pl';
require "${General::swroot}/lang.pl";
require "${General::swroot}/header.pl";

my %color = ();
my %mainsettings = ();
my %mpfiresettings = ();
my %checked = ();
my $message = "";
my $errormessage = "";

open(DATEI, "<${General::swroot}/mpfire/db/songs.db") || die "No Database found";
my @songdb = <DATEI>;
close(DATEI);
@songdb = sort(@songdb);

&General::readhash("${General::swroot}/main/settings", \%mainsettings);
&General::readhash("/srv/web/ipfire/html/themes/".$mainsettings{'THEME'}."/include/colors.txt", \%color);

&Header::showhttpheaders();
&Header::getcgihash(\%mpfiresettings);

&Header::openpage($Lang::tr{'mpfire'}, 1, '');
&Header::openbigbox('100%', 'left', '', $errormessage);

sub refreshpage{&Header::openbox( 'Waiting', 1, "<meta http-equiv='refresh' content='1;'>" );}

############################################################################################################################
######################################## Scanne Verzeichnisse nach Mp3 Dateien #############################################

if ( $mpfiresettings{'ACTION'} eq "scan" )
{
delete $mpfiresettings{'__CGI__'};delete $mpfiresettings{'x'};delete $mpfiresettings{'y'};
&General::writehash("${General::swroot}/mpfire/settings", \%mpfiresettings);
system("/usr/local/bin/mpfirectrl scan $mpfiresettings{'SCANDIR'} $mpfiresettings{'SCANDIRDEPS'}");
}

if ( $mpfiresettings{'ACTION'} eq ">" ){system("/usr/local/bin/mpfirectrl play $mpfiresettings{'FILE'}");}
if ( $mpfiresettings{'ACTION'} eq "x" ){system("/usr/local/bin/mpfirectrl stop");}
if ( $mpfiresettings{'ACTION'} eq "||" ){system("/usr/local/bin/mpfirectrl pause");}
if ( $mpfiresettings{'ACTION'} eq "|>" ){system("/usr/local/bin/mpfirectrl resume");}
if ( $mpfiresettings{'ACTION'} eq ">>" ){system("/usr/local/bin/mpfirectrl next");}
if ( $mpfiresettings{'ACTION'} eq "+" ){system("/usr/local/bin/mpfirectrl volup 5");}
if ( $mpfiresettings{'ACTION'} eq "-" ){system("/usr/local/bin/mpfirectrl voldown 5");}
if ( $mpfiresettings{'ACTION'} eq "++" ){system("/usr/local/bin/mpfirectrl volup 10");}
if ( $mpfiresettings{'ACTION'} eq "--" ){system("/usr/local/bin/mpfirectrl voldown 10");}
if ( $mpfiresettings{'ACTION'} eq "playall" )
{
my @temp = "";
foreach (@songdb){
  my @song = split(/\|/,$_);
  chomp($song[0]);
  push(@temp,$song[0]."\n");
  }
open(DATEI, ">${General::swroot}/mpfire/playlist") || die "Could not add playlist";
print DATEI @temp;
close(DATEI);
system("/usr/local/bin/mpfirectrl playall");
}
if ( $mpfiresettings{'SHOWLIST'} ){delete $mpfiresettings{'__CGI__'};delete $mpfiresettings{'x'};delete $mpfiresettings{'y'};&General::writehash("${General::swroot}/mpfire/settings", \%mpfiresettings);}

############################################################################################################################
################################### Aufbau der HTML Seite fr globale Sambaeinstellungen ####################################

$mpfiresettings{'SCANDIR'} = "/";
$mpfiresettings{'SHOWLIST'} = "off";

&General::readhash("${General::swroot}/mpfire/settings", \%mpfiresettings);

############################################################################################################################
########################################### rekursiv nach neuen Mp3s Scannen ##############################################ä

if ( $message ne "" )	{	print "<font color='red'>$message</font>"; }

&Header::openbox('100%', 'center', $Lang::tr{'mpfire scanning'});
	
print <<END
<form method='post' action='$ENV{'SCRIPT_NAME'}'>
<table width='95%' cellspacing='0'>
<tr bgcolor='$color{'color20'}'><td colspan='2' align='left'><b>$Lang::tr{'Scan for Files'}</b></td></tr>
<tr><td align='left' width='40%'>$Lang::tr{'Scan from Directory'}</td><td align='left'><input type='text' name='SCANDIR' value='$mpfiresettings{'SCANDIR'}' size="30" /></td></tr>
<tr><td align='left' width='40%'>$Lang::tr{'deep scan directories'}</td><td align='left'>on <input type='radio' name='SCANDIRDEPS' value='on' checked='checked'/>/
																									                                          <input type='radio' name='SCANDIRDEPS' value='off'/> off</td></tr>
<tr><td align='center' colspan='2'><input type='hidden' name='ACTION' value='scan' />
                              <input type='image' alt='$Lang::tr{'Scan for Files'}' title='$Lang::tr{'Scan for Files'}' src='/images/edit-find.png' /></td></tr>																				
</table>
</form>
END
;
&Header::closebox();

&Header::openbox('100%', 'center', $Lang::tr{'mpfire controls'});
print "<form method='post' action='$ENV{'SCRIPT_NAME'}'><table width='95%' cellspacing='0'><tr>";
print <<END
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='x' /><input type='image' alt='$Lang::tr{'stop'}' title='$Lang::tr{'stop'}' src='/images/media-playback-stop.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='||' /><input type='image' alt='$Lang::tr{'pause'}' title='$Lang::tr{'pause'}' src='/images/media-playback-pause.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='|>' /><input type='image' alt='$Lang::tr{'resume'}' title='$Lang::tr{'resume'}' src='/images/media-resume.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='playall' /><input type='image' alt='$Lang::tr{'play'}' title='$Lang::tr{'play'}' src='/images/media-playback-start.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='>>' /><input type='image' alt='$Lang::tr{'next'}' title='$Lang::tr{'next'}' src='/images/media-skip-forward.png' /></form></td>
    </tr>
END
;
if ( $mpfiresettings{'SHOWLIST'} eq "on" ){print"<tr><td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='SHOWLIST' value='off' /><input type='image' alt='$Lang::tr{'off'}' title='$Lang::tr{'off'}' src='/images/audio-x-generic.png' /></form></td>";}
else { print"<tr><td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='SHOWLIST' value='on' /><input type='image' alt='$Lang::tr{'on'}' title='$Lang::tr{'on'}' src='/images/audio-x-generic-red.png' /></form></td>";}    
print <<END  
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='--' /><input type='image' alt='$Lang::tr{'voldown10'}' title='$Lang::tr{'voldown10'}' src='/images/audio-volume-low-red.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='-' /><input type='image' alt='$Lang::tr{'voldown5'}' title='$Lang::tr{'voldown5'}' src='/images/audio-volume-low.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='+' /><input type='image' alt='$Lang::tr{'volup5'}' title='$Lang::tr{'volup5'}' src='/images/audio-volume-high.png' /></form></td>
    <td align='center'><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='++' /><input type='image' alt='$Lang::tr{'volup10'}' title='$Lang::tr{'volup10'}' src='/images/audio-volume-high-red.png' /></form></td>
    </tr>
</table>
END
;
&Header::closebox();

if ( $mpfiresettings{'SHOWLIST'} eq "on" ){

&Header::openbox('100%', 'center', $Lang::tr{'mpfire songs'});
print <<END

<table width='95%' cellspacing=5'>
<tr bgcolor='$color{'color20'}'><td colspan='9' align='left'><b>$Lang::tr{'Existing Files'}</b></td></tr>
<tr><td align='center'></td>
    <td align='center'><b>$Lang::tr{'artist'}<br/>$Lang::tr{'title'}</b></td>
    <td align='center'><b>$Lang::tr{'number'}</b></td>
    <td align='center'><b>$Lang::tr{'album'}</b></td>
    <td align='center'><b>$Lang::tr{'year'}</b></td>
    <td align='center'><b>$Lang::tr{'genre'}</b></td>
    <td align='center'><b>$Lang::tr{'length'}<br/>$Lang::tr{'bitrate'} - $Lang::tr{'frequency'}</b></td>
    <td align='center'><b>$Lang::tr{'mode'}</b></td></tr>
END
;
my $lines = 0;
foreach (@songdb){
  my @song = split(/\|/,$_);
  
  if ($lines % 2) {print "<tr bgcolor='$color{'color20'}'>";} else {print "<tr bgcolor='$color{'color22'}'>";}
  $song[0]=~s/\/\//\//g;   
  print <<END
  <td align='center' style="white-space:nowrap;"><form method='post' action='$ENV{'SCRIPT_NAME'}'><input type='hidden' name='ACTION' value='>' /><input type='hidden' name='FILE' value='$song[0]' /><input type='image' alt='$Lang::tr{'play'}' title='$Lang::tr{'play'}' src='/images/media-playback-start.png' /></form></td>
  <td align='center'>$song[1]<br/>$song[2]</td>
  <td align='center'>$song[3]</td>
  <td align='center'>$song[4]</td>
  <td align='center'>$song[5]</td>
  <td align='center'>$song[6]</td>
  <td align='center'>$song[7]:$song[8]<br/>$song[9] - $song[10]</td>
END
;
    if ( $song[11] eq "0\n" ) {print "<td align='center'>Stereo</td></tr>"; }
    elsif ( $song[11] eq "1\n" ) {print "<td align='center'>Joint<br/>Stereo</td></tr>"; }
    elsif ( $song[11] eq "2\n" ) {print "<td align='center'>Dual<br/>Channel</td></tr>"; }
    elsif ( $song[11] eq "3\n" ) {print "<td align='center'>Single<br/>Channel</td></tr>"; }
    else {print "<td align='center'></td></tr>"; }
  $lines++
  }
print "</table></form>";
&Header::closebox();
}

&Header::closebigbox();
&Header::closepage();
