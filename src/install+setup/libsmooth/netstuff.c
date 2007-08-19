/* SmoothWall libsmooth.
 *
 * This program is distributed under the terms of the GNU General Public
 * Licence.  See the file COPYING for details.
 *
 * (c) Lawrence Manning, 2001
 * Contains network library functions.
 * 
 */

#include "libsmooth.h"
#include <signal.h>

extern FILE *flog;
extern char *mylog;

extern char **ctr;

extern struct nic nics[];
extern struct knic knics[];

char *ucolourcard[] = { "GREEN", "RED", "ORANGE", "BLUE", NULL };
char *lcolourcard[] = { "green", "red", "orange", "blue", NULL };

int scanned_nics_read_done = 0;

newtComponent networkform;
newtComponent addressentry;
newtComponent netmaskentry;
newtComponent statictyperadio;
newtComponent dhcptyperadio;
newtComponent pppoetyperadio;
newtComponent pptptyperadio;
newtComponent dhcphostnameentry;

/* acceptable character filter for IP and netmaks entry boxes */
static int ip_input_filter(newtComponent entry, void * data, int ch, int cursor)
{
	if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '\r' || ch >= NEWT_KEY_EXTRA_BASE)
		return ch;
	return 0;
}

/* This is a groovie dialog for showing network info.  Takes a keyvalue list,
 * a colour and a dhcp flag.  Shows the current settings, and rewrites them
 * if necessary.  DHCP flag sets wether to show the dhcp checkbox. */
int changeaddress(struct keyvalue *kv, char *colour, int typeflag,
	char *defaultdhcphostname)
{
	char *addressresult;
	char *netmaskresult;
	char *dhcphostnameresult;
	struct newtExitStruct es;
	newtComponent header;
	newtComponent addresslabel;
	newtComponent netmasklabel;
	newtComponent dhcphostnamelabel;
	newtComponent ok, cancel;	
	char message[1000];
	char temp[STRING_SIZE];
	char addressfield[STRING_SIZE];
	char netmaskfield[STRING_SIZE];
	char typefield[STRING_SIZE];
	char dhcphostnamefield[STRING_SIZE];
	int error;
	int result = 0;
	char type[STRING_SIZE];
	int startstatictype = 0;
	int startdhcptype = 0;
	int startpppoetype = 0;
	int startpptptype = 0;
		
	/* Build some key strings. */
	sprintf(addressfield, "%s_ADDRESS", colour);
	sprintf(netmaskfield, "%s_NETMASK", colour);
	sprintf(typefield, "%s_TYPE", colour);
	sprintf(dhcphostnamefield, "%s_DHCP_HOSTNAME", colour);
		
	sprintf(message, ctr[TR_INTERFACE], colour);
	newtCenteredWindow(44, (typeflag ? 18 : 12), message);
	
	networkform = newtForm(NULL, NULL, 0);

	sprintf(message, ctr[TR_ENTER_THE_IP_ADDRESS_INFORMATION], colour);
	header = newtTextboxReflowed(1, 1, message, 42, 0, 0, 0);
	newtFormAddComponent(networkform, header);

	/* See if we need a dhcp checkbox.  If we do, then we shift the contents
	 * of the window down two rows to make room. */
	if (typeflag)
	{
		strcpy(temp, "STATIC"); findkey(kv, typefield, temp);
		if (strcmp(temp, "STATIC") == 0) startstatictype = 1;
		if (strcmp(temp, "DHCP") == 0) startdhcptype = 1;
		if (strcmp(temp, "PPPOE") == 0) startpppoetype = 1;
		if (strcmp(temp, "PPTP") == 0) startpptptype = 1;
		statictyperadio = newtRadiobutton(2, 4, ctr[TR_STATIC], startstatictype, NULL);
		dhcptyperadio = newtRadiobutton(2, 5, "DHCP", startdhcptype, statictyperadio);
		pppoetyperadio = newtRadiobutton(2, 6, "PPPOE", startpppoetype, dhcptyperadio);
		pptptyperadio = newtRadiobutton(2, 7, "PPTP", startpptptype, pppoetyperadio);
		newtFormAddComponents(networkform, statictyperadio, dhcptyperadio, 
			pppoetyperadio, pptptyperadio, NULL);
		newtComponentAddCallback(statictyperadio, networkdialogcallbacktype, NULL);
		newtComponentAddCallback(dhcptyperadio, networkdialogcallbacktype, NULL);
		newtComponentAddCallback(pppoetyperadio, networkdialogcallbacktype, NULL);
		newtComponentAddCallback(pptptyperadio, networkdialogcallbacktype, NULL);
		dhcphostnamelabel = newtTextbox(2, 9, 18, 1, 0);
		newtTextboxSetText(dhcphostnamelabel, ctr[TR_DHCP_HOSTNAME]);
		strcpy(temp, defaultdhcphostname);
		findkey(kv, dhcphostnamefield, temp);
		dhcphostnameentry = newtEntry(20, 9, temp, 20, &dhcphostnameresult, 0);
		newtFormAddComponent(networkform, dhcphostnamelabel);		
		newtFormAddComponent(networkform, dhcphostnameentry);	
		if (startdhcptype == 0)
			newtEntrySetFlags(dhcphostnameentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);
	}
	/* Address */
	addresslabel = newtTextbox(2, (typeflag ? 11 : 4) + 0, 18, 1, 0);
	newtTextboxSetText(addresslabel, ctr[TR_IP_ADDRESS_PROMPT]);
	strcpy(temp, "");
	findkey(kv, addressfield, temp);
	addressentry = newtEntry(20, (typeflag ? 11 : 4) + 0, temp, 20, &addressresult, 0);
	newtEntrySetFilter(addressentry, ip_input_filter, NULL);
	if (typeflag == 1 && startstatictype == 0 && startpptptype == 0 )
		newtEntrySetFlags(addressentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);
	newtFormAddComponent(networkform, addresslabel);
	newtFormAddComponent(networkform, addressentry);
	
	/* Netmask */
	netmasklabel = newtTextbox(2, (typeflag ? 11 : 4) + 1, 18, 1, 0);
	newtTextboxSetText(netmasklabel, ctr[TR_NETMASK_PROMPT]);
	strcpy(temp, "255.255.255.0"); findkey(kv, netmaskfield, temp);
	netmaskentry = newtEntry(20, (typeflag ? 11 : 4) + 1, temp, 20, &netmaskresult, 0);
	newtEntrySetFilter(netmaskentry, ip_input_filter, NULL);
	if (typeflag == 1 && startstatictype == 0 && startpptptype == 0 ) 
		newtEntrySetFlags(netmaskentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);

	newtFormAddComponent(networkform, netmasklabel);
	newtFormAddComponent(networkform, netmaskentry);

	/* Buttons. */
	ok = newtButton(8, (typeflag ? 14 : 7), ctr[TR_OK]);
	cancel = newtButton(26, (typeflag ? 14 : 7), ctr[TR_CANCEL]);

	newtFormAddComponents(networkform, ok, cancel, NULL);

	newtRefresh();
	newtDrawForm(networkform);

	do
	{
		error = 0;
		newtFormRun(networkform, &es);
	
		if (es.u.co == ok)
		{
			/* OK was pressed; verify the contents of each entry. */
			strcpy(message, ctr[TR_INVALID_FIELDS]);
			
			strcpy(type, "STATIC");
			if (typeflag)
				gettype(type);
			if (strcmp(type, "STATIC") == 0 || strcmp(type, "PPTP") == 0 )
			{		
				if (inet_addr(addressresult) == INADDR_NONE)
				{
					strcat(message, ctr[TR_IP_ADDRESS_CR]);
					error = 1;
				}
				if (inet_addr(netmaskresult) == INADDR_NONE)
				{
					strcat(message, ctr[TR_NETWORK_MASK_CR]);
					error = 1;
				}
			}
			if (strcmp(type, "DHCP") == 0)
			{
				if (!strlen(dhcphostnameresult))
				{
					strcat(message, ctr[TR_DHCP_HOSTNAME_CR]);
					error = 1;
				}
			}
			if (error)
				errorbox(message);
			else
			{
				/* No errors!  Set new values, depending on dhcp flag etc. */
				if (typeflag)
				{
					replacekeyvalue(kv, dhcphostnamefield, dhcphostnameresult);
					if (strcmp(type, "STATIC") != 0 && strcmp(type, "PPTP") != 0)
					{
						replacekeyvalue(kv, addressfield, "0.0.0.0");
						replacekeyvalue(kv, netmaskfield, "0.0.0.0");
					}
					else
					{
						replacekeyvalue(kv, addressfield, addressresult);
						replacekeyvalue(kv, netmaskfield, netmaskresult);
					}
					replacekeyvalue(kv, typefield, type);					
				}
				else
				{
					replacekeyvalue(kv, addressfield, addressresult);
					replacekeyvalue(kv, netmaskfield, netmaskresult);
				}
				
				setnetaddress(kv, colour);
				result = 1;
			}
		}			
	}
	while (error);

	newtFormDestroy(networkform);
	newtPopWindow();
		
	return result;
}

/* for pppoe: return string thats type STATIC, DHCP or PPPOE */
int gettype(char *type)
{
	newtComponent selected = newtRadioGetCurrent(statictyperadio);
	
	if (selected == statictyperadio)
		strcpy(type, "STATIC");
	else if (selected == dhcptyperadio)
		strcpy(type, "DHCP");
	else if (selected == pppoetyperadio)
		strcpy(type, "PPPOE");
	else if (selected == pptptyperadio)
		strcpy(type, "PPTP");
	else
		strcpy(type, "ERROR");
	
	return 0;
}

/* 0.9.9: calculates broadcast too. */
int setnetaddress(struct keyvalue *kv, char *colour)
{
	char addressfield[STRING_SIZE];
	char netaddressfield[STRING_SIZE];		
	char netmaskfield[STRING_SIZE];
	char broadcastfield[STRING_SIZE];
	char address[STRING_SIZE];
	char netmask[STRING_SIZE];
	unsigned long int intaddress;
	unsigned long int intnetaddress;
	unsigned long int intnetmask;
	unsigned long int intbroadcast;
	struct in_addr temp;
	char *netaddress;
	char *broadcast;
		
	/* Build some key strings. */
	sprintf(addressfield, "%s_ADDRESS", colour);
	sprintf(netaddressfield, "%s_NETADDRESS", colour);
	sprintf(netmaskfield, "%s_NETMASK", colour);
	sprintf(broadcastfield, "%s_BROADCAST", colour);

	strcpy(address, ""); findkey(kv, addressfield, address);	
	strcpy(netmask, ""); findkey(kv, netmaskfield, netmask);		

	/* Calculate netaddress. Messy.. */
	intaddress = inet_addr(address);
	intnetmask = inet_addr(netmask);
	
	intnetaddress = intaddress & intnetmask;
	temp.s_addr = intnetaddress;	
	netaddress = inet_ntoa(temp);
	
	replacekeyvalue(kv, netaddressfield, netaddress);
	
	intbroadcast = intnetaddress | ~intnetmask;
	temp.s_addr = intbroadcast;
	broadcast = inet_ntoa(temp);	
	
	replacekeyvalue(kv, broadcastfield, broadcast);
	
	return 1;
}	

/* Called when dhcp flag is toggled.  Toggle disabled state of other 3
 * controls. */
void networkdialogcallbacktype(newtComponent cm, void *data)
{
	char type[STRING_SIZE];
	
	gettype(type);

	if (strcmp(type, "STATIC") != 0  && strcmp(type, "PPTP") != 0 )
	{
		newtEntrySetFlags(addressentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);
		newtEntrySetFlags(netmaskentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);
	}
	else
	{
		newtEntrySetFlags(addressentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_RESET);
		newtEntrySetFlags(netmaskentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_RESET);
	}
	if (strcmp(type, "DHCP") == 0)
		newtEntrySetFlags(dhcphostnameentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_RESET);
	else
		newtEntrySetFlags(dhcphostnameentry, NEWT_FLAG_DISABLED, NEWT_FLAGS_SET);		
		
	newtRefresh();
	newtDrawForm(networkform);	
}

int interfacecheck(struct keyvalue *kv, char *colour)
{
	char temp[STRING_SIZE];
	char colourfields[NETCHANGE_TOTAL][STRING_SIZE];
	int c;

	sprintf(colourfields[ADDRESS], "%s_ADDRESS", colour);
	sprintf(colourfields[NETADDRESS], "%s_NETADDRESS", colour);
	sprintf(colourfields[NETMASK], "%s_NETMASK", colour);

	for (c = 0; c < 3; c++)
	{
		strcpy(temp, ""); findkey(kv, colourfields[c], temp);
		if (!(strlen(temp))) return 0;
	}
	return 1;
}

/* Funky routine for loading all drivers (cept those are already loaded.). */
int probecards(char *driver, char *driveroptions )
{
	return 0;
}

/* ### alter strupper ###
char *strupper(char *s)
{
 int n;
 for (n=0;s[n];n++) s[n]=toupper(s[n]);
 return s;
}
*/

/* neuer StringUpper, wird zur Zeit nicht benutzt da UTF-8 nicht geht.
void strupper(unsigned char *string)
{
	unsigned char *str;
	for (str = string; *str != '\0'; str++)
		if (!(*str & 0x80) && islower(*str)) 
			*str = toupper(*str);
}
*/

/* int ismacaddr(char *ismac)
{
	char *a;
	fprintf(flog,"Check is MAC true\n"); // #### Debug ####
	for (a = ismac; *a; a++) {
		sprintf(flog,"%c\n", *a);	// #### Debug ####
		if (*a != ':' && !isxdigit(*a)) return 0;	// is int != ':' or not hexdigit then exit
	}
	return 1;
}
*/

int get_knic(int card)		//returns "0" for zero cards or error and "1" card is found.
{
	struct keyvalue *kv = initkeyvalues();
	char temp[STRING_SIZE], searchstr[STRING_SIZE];
	int ret_value;

	if (!(readkeyvalues(kv, CONFIG_ROOT "/ethernet/settings")))
	{
		freekeyvalues(kv);
		errorbox(ctr[TR_UNABLE_TO_OPEN_SETTINGS_FILE]);
		return 0;
	}

	sprintf(searchstr, "%s_MACADDR", ucolourcard[card]);
	strcpy(temp, ""); findkey(kv, searchstr, temp);
	if (strlen(temp)) {
		strcpy(knics[ card ].macaddr, temp);
		strcpy(knics[ card ].colour, "GREEN");

		sprintf(searchstr, "%s_DESCRIPTION", ucolourcard[card]);
		findkey(kv, searchstr, temp);
		strcpy(knics[ card ].description, temp);

		sprintf(searchstr, "%s_DRIVER", ucolourcard[card]);
		findkey(kv, searchstr, temp);
		strcpy(knics[ card ].driver, temp);
		ret_value = 1;
	} else {
		strcpy(knics[ card ].description, ctr[TR_UNSET]);
		ret_value = 0;
	}

	return ret_value;
}

int init_knics(void)
{
	int found = 0;
	found += get_knic(_GREEN_CARD_);
	found += get_knic(_RED_CARD_);
	found += get_knic(_ORANGE_CARD_);
	found += get_knic(_BLUE_CARD_);

	return found;
}

int fmt_exists(const char *fname) {	/* Check it's any File or Directory */
	struct stat st;
	if (stat(fname, &st) == -1) return 0;
	else return 1;
}

int is_interface_up(char *card) {	/* Check is interface UP */
	char temp[STRING_SIZE];

	sprintf(temp,"ip link show dev %s | grep -q UP", card);
	if (mysystem(temp)) return 0; else return 1;
}

int rename_device(char *old_name, char *new_name) {
	char temp[STRING_SIZE];

	sprintf(temp,SYSDIR "/%s", old_name);
	if (!(fmt_exists(temp))) {
		fprintf(flog,"Device not found: %s\n",old_name);
		return 0;
	}
//	fprintf(flog,"NIC: %s wurde in %s umbenannt.\n", old_name, new_name);	// #### Debug ####
	sprintf(temp,"/sbin/ip link set dev %s name %s",old_name ,new_name );
	mysystem(temp);

	return 1;
}

char g_temp[STRING_SIZE]="";
char* readmac(char *card) {
//	fprintf(flog,"Enter readmac... NIC: %s\n", card);	// #### Debug ####
	FILE *fp;
	char temp[STRING_SIZE], mac[20];

	sprintf(temp,"/sys/class/net/%s/address",card);
	if( (fp = fopen(temp, "r")) == NULL ) {
		fprintf(flog,"Couldn't open: %s\n",temp);
		return NULL;
	}
	fgets(mac, 18, fp);
	strtok(mac,"\n");
	fclose(fp);
	strcpy(g_temp, mac);
	return g_temp;
}

char* find_nic4mac(char *findmac) {
	fprintf(flog,"Enter find_name4nic... Search for %s\n", findmac);	// #### Debug ####

	DIR *dir;
	struct dirent *dirzeiger;
	char temp[STRING_SIZE], temp2[STRING_SIZE];
        
	if((dir=opendir(SYSDIR)) == NULL) {
		fprintf(flog,"Fehler bei opendir (find_name4nic) ...\n");
		return NULL;
	}

	sprintf(temp, "");
	while((dirzeiger=readdir(dir)) != NULL) {
		if(*((*dirzeiger).d_name) != '.' & strcmp(((*dirzeiger).d_name), "lo") != 0) {
			sprintf(temp2, "%s", readmac((*dirzeiger).d_name) );
			if (strcmp(findmac, temp2) == 0) {
				sprintf(temp,"%s", (*dirzeiger).d_name);
//				fprintf(flog,"MAC: %s is NIC: %s\n", findmac, temp);	// #### Debug ####
				break;
			}
		}
	}

	if(closedir(dir) == -1) fprintf(flog,"Fehler beim schliessen von %s\n", SYSDIR);
	strcpy(g_temp, temp);
	return g_temp;
}

int nic_shutdown(char *nic) {
	char temp[STRING_SIZE];
	
	sprintf(temp,"ip link set %s down", nic);
	mysystem(temp);
}

int nic_startup(char *nic) {
	char temp[STRING_SIZE];
	
	sprintf(temp,"ip link set %s up", nic);
	mysystem(temp);

}

int rename_nics(void) {
	int i, j, k;
	int fnics = scan_network_cards();
	char nic2find[STRING_SIZE], temp[STRING_SIZE];

	fprintf(flog,"Renaming Nics\n");	// #### Debug ####

	for(i=0; i<4; i++)
		if (strcmp(knics[i].macaddr, ""))									// Wird das Interface benutzt ?
			for(j=0; j<fnics; j++)
				if(strcmp(knics[i].macaddr, nics[j].macaddr) == 0) {					// suche den aktuellen Namen
					sprintf(nic2find,"%s0",lcolourcard[i]);
//					fprintf(flog,"search4: %s\n", nic2find);	// #### Debug ####
					if(strcmp(nic2find, nics[j].nic)) {						// hat das Interface nicht den Namen ?
//						fprintf(flog,"cmp nic2find false\n");	// #### Debug ####
//						fprintf(flog,"is nic( %s ) up ?\n",nics[j].nic);	// #### Debug ####

						if(is_interface_up(nics[j].nic)) {					// wurde das Interface gestartet ?
//							fprintf(flog,"%s is UP, shutting down...\n",nics[j].nic);	// #### Debug ####
							nic_shutdown(nics[j].nic);
						}
						sprintf(temp,SYSDIR "/%s", nic2find);
//						fprintf(flog,"exists ?--> %s\n", temp);	// #### Debug ####
						if(fmt_exists(temp)) {							// Ist der Name schon in Benutzung ?
//							fprintf(flog,"is exists %s\n", nic2find);	// #### Debug ####
							for(k=0; k<fnics; k++) 				// Suche das Interface
								if (strcmp(nics[k].nic, nic2find) == 0 ) {
									if(is_interface_up(nics[k].nic)) {		// wurde das Interface gestartet ?
//										fprintf(flog,"%s is UP, shutting down...\n",nics[k].nic);	// #### Debug ####
										nic_shutdown(nics[k].nic);
									}
									sprintf(temp,"dummy%i",k);			// Benenne NIC nach "dummy[k]" um.
//									fprintf(flog,"set dummy%i\n", k);	// #### Debug ####
									if (rename_device(nics[k].nic, temp)) strcpy(nics[k].nic, temp);
								}
						}
						if (rename_device(nics[j].nic, nic2find)) strcpy(nics[j].nic, nic2find);	// Benenne NIC um.

//						if(strncmp(nics[j].nic,"dummy",5)) {
//							fprintf(flog,"%s is down, start up...\n",nics[j].nic);	// #### Debug ####
//							nic_startup(nics[j].nic);
//						}
					}
				}
}

int create_udev(void)
{
	#define UDEV_NET_CONF "/etc/udev/rules.d/30-persistent-network.rules"
	FILE *fp;
	int i;

	fprintf(flog,"Enter create_udev: "UDEV_NET_CONF"\n"); // #### Debug ####
	if ( (fp = fopen(UDEV_NET_CONF, "w")) == NULL ) {
		fprintf(stderr,"Couldn't open" UDEV_NET_CONF);
		return 1;
	}

	for (i = 0 ; i < 4 ; i++)
	{
		if (strcmp(knics[i].macaddr, "")) {
			fprintf(fp,"ACTION==\"add\", SUBSYSTEM==\"net\", SYSFS{address}==\"%s\", NAME=\"%s0\" # %s\n", knics[i].macaddr, lcolourcard[i], knics[i].description);
			fprintf(flog,"Write %s\n",lcolourcard[i]); // #### Debug ####
		}
	}
	fclose(fp);
	return 0;
}

int write_configs_netudev(int card , int colour)
{	
	char commandstring[STRING_SIZE];
	struct keyvalue *kv = initkeyvalues();
	char temp1[STRING_SIZE], temp2[STRING_SIZE], temp3[STRING_SIZE];
	char ucolour[STRING_SIZE];

	sprintf(ucolour, ucolourcard[colour]);
	strcpy(knics[colour].driver, nics[card].driver);
	strcpy(knics[colour].description, nics[card].description);
	strcpy(knics[colour].macaddr, nics[card].macaddr);
	
	if (!(readkeyvalues(kv, CONFIG_ROOT "/ethernet/settings")))
	{
		freekeyvalues(kv);
		errorbox(ctr[TR_UNABLE_TO_OPEN_SETTINGS_FILE]);
		return 0;
	}

	sprintf(temp1, "%s_DEV", ucolour);
	sprintf(temp2, "%s_MACADDR", ucolour);
	sprintf(temp3, "%s0", lcolourcard[colour]);
	replacekeyvalue(kv, temp1, temp3);
	replacekeyvalue(kv, temp2, nics[card].macaddr);
	sprintf(temp1, "%s_DESCRIPTION", ucolour);
	replacekeyvalue(kv, temp1, nics[card].description);
	sprintf(temp1, "%s_DRIVER", ucolour);
	replacekeyvalue(kv, temp1, nics[card].driver);

	writekeyvalues(kv, CONFIG_ROOT "/ethernet/settings");
	freekeyvalues(kv);
	
	return 0;
}

int scan_network_cards(void)
{
	FILE *fp;
	char driver[STRING_SIZE], description[STRING_SIZE], macaddr[STRING_SIZE], temp_line[STRING_SIZE];
	int count = 0;
	const char _driver[]="driver: ";
	const char _desc[]="desc: ";
	const char _network_hwaddr[]="network.hwaddr: ";
	
	if (!(scanned_nics_read_done))
	{
		fprintf(flog,"Enter scan_network_cards\n"); // #### Debug ####
		mysystem("/bin/probenic.sh");
		// Read our scanned nics
		if( (fp = fopen(SCANNED_NICS, "r")) == NULL )
		{
			fprintf(stderr,"Couldn't open "SCANNED_NICS);
			return 1;
		}
		while (fgets(temp_line, STRING_SIZE, fp) != NULL)
		{
			temp_line[strlen(temp_line) -1] = 0;
			if ( strncmp(temp_line, _driver,         strlen(_driver))         ==  0 ) sprintf(nics[count].driver,      "%s", temp_line+strlen(_driver));
			if ( strncmp(temp_line, _desc,           strlen(_desc))           ==  0 ) sprintf(nics[count].description, "%s", temp_line+strlen(_desc));
			if ( strncmp(temp_line, _network_hwaddr, strlen(_network_hwaddr)) ==  0 ) sprintf(nics[count].macaddr,     "%s", temp_line+strlen(_network_hwaddr));
			if (strlen(nics[count].macaddr) > 15 ) {
				sprintf(nics[count].nic, "%s", find_nic4mac(nics[count].macaddr));
				count++;
			}
		}
		fclose(fp);
		scanned_nics_read_done = count;
	} else fprintf(flog,"Scan Networkcards does read.\n");
	return scanned_nics_read_done;
}



int nicmenu(int colour)
{
	int rc, choise = 0, count = 0, kcount = 0, mcount = 0, i, j, nic_in_use;
	int found_NIC_as_Card[4];
	char message[STRING_SIZE];

	char cMenuInhalt[STRING_SIZE];
	char MenuInhalt[20][180];
	char *pMenuInhalt[20];
	
//	strcpy( message , pnics[count].macaddr);
//	while (strcmp(message, "")) {
//		count++;
//		strcpy( message , pnics[count].macaddr);
//	}

	while (strcmp(nics[count].macaddr, "")) count++;			// 2 find how many nics in system
	for ( i=0 ; i<4;i++) if (strcmp(knics[i].macaddr, "")) kcount++;	// loop to find all knowing nics
	fprintf(flog, "Enter NicMenu\n");					// #### Debug ####
	fprintf(flog, "count nics %i\ncount knics %i\n", count, kcount);	// #### Debug ####

	// If new nics are found...
	if (count > kcount) {
		for (i=0 ; i < count ; i++)
		{
			nic_in_use = 0;
			for (j=0 ; j <= kcount ; j++) {
				if (strcmp(nics[ i ].macaddr, knics[ j ].macaddr) == 0 ) {
					nic_in_use = 1;
					fprintf(flog,"NIC \"%s\" is in use.\n", nics[ i ].macaddr); // #### Debug ####
					break;
				}
			}
			if (!(nic_in_use)) {
				fprintf(flog,"NIC \"%s\" is free.\n", nics[ i ].macaddr); // #### Debug ####
				if ( strlen(nics[i].description) < 55 ) 
					sprintf(MenuInhalt[mcount], "%.*s",  strlen(nics[i].description)-2, nics[i].description+1);
				else {
					sprintf(cMenuInhalt, "%.50s", nics[i].description + 1);
					sprintf(MenuInhalt[mcount], cMenuInhalt);
					strcat (MenuInhalt[mcount], "...");
				}

				while ( strlen(MenuInhalt[mcount]) < 53) strcat(MenuInhalt[mcount], " "); // Fill with space.

				strcat(MenuInhalt[mcount], " (");
				strcat(MenuInhalt[mcount], nics[i].macaddr);
				strcat(MenuInhalt[mcount], ")");
				pMenuInhalt[mcount] = MenuInhalt[mcount];
				found_NIC_as_Card[mcount]=i;
				mcount++;
			}
		}

		pMenuInhalt[mcount] = NULL;

//		sprintf(message, "Es wurde(n) %d freie Netzwerkkarte(n) in Ihrem System gefunden.\nBitte waehlen Sie im naechsten Dialog eine davon aus.\n", count);
//		newtWinMessage("NetcardMenu", ctr[TR_OK], message);

		sprintf(message, ctr[TR_CHOOSE_NETCARD], ucolourcard[colour]);
		rc = newtWinMenu( ctr[TR_NETCARDMENU2], message, 50, 5, 5, 6, pMenuInhalt, &choise, ctr[TR_OK], ctr[TR_SELECT], ctr[TR_CANCEL], NULL);
				
		if ( rc == 0 || rc == 1) {
			write_configs_netudev(found_NIC_as_Card[choise], colour);
		} else if (rc == 2) {
//			manualdriver("pcnet32","");
	      	}
		return 0;
	} else {
		// We have to add here that you can manually add a device
		errorbox( ctr[TR_ERROR_INTERFACES]);
		return 1;
	}
}

int clear_card_entry(int card)
{
	struct keyvalue *kv = initkeyvalues();
	char temp[STRING_SIZE];

	if (!(readkeyvalues(kv, CONFIG_ROOT "/ethernet/settings")))
	{
		freekeyvalues(kv);
		errorbox(ctr[TR_UNABLE_TO_OPEN_SETTINGS_FILE]);
		return 0;
	}

	strcpy(knics[card].driver, "");
	strcpy(knics[card].description, ctr[TR_UNSET]);
	strcpy(knics[card].macaddr, "");
	strcpy(knics[card].colour, "");
	sprintf(temp, "%s_DRIVER", ucolourcard[card]);
	replacekeyvalue(kv, temp, "");
	sprintf(temp, "%s_DEV", ucolourcard[card]);
	replacekeyvalue(kv, temp, "");
	sprintf(temp, "%s_MACADDR", ucolourcard[card]);
	replacekeyvalue(kv, temp, "");
	sprintf(temp, "%s_DESCRIPTION", ucolourcard[card]);
	replacekeyvalue(kv, temp, "");

	writekeyvalues(kv, CONFIG_ROOT "/ethernet/settings");
	freekeyvalues(kv);

	fprintf(flog,"Card \"%s\" cleared\n",ucolourcard[card]); // #### Debug ####
	return 0;
}

int ask_clear_card_entry(int card)
{
	char message[STRING_SIZE];
	int rc;

	sprintf(message, ctr[TR_REMOVE_CARD] "%s \n", ucolourcard[card]);
	rc = newtWinChoice(ctr[TR_WARNING], ctr[TR_OK], ctr[TR_CANCEL], message);				

	if ( rc = 0 || rc == 1) {
		clear_card_entry(card);
//		sprintf(temp1, "%s_DEV", ucolour);
//		sprintf(temp2, "%s_MACADDR", ucolour);
//		replacekeyvalue(kv, temp1, "");
//		replacekeyvalue(kv, temp2, "");
//		sprintf(temp1, "%s_DESCRIPTION", ucolour);
//		replacekeyvalue(kv, temp1, "");

//		writekeyvalues(kv, CONFIG_ROOT "/ethernet/settings");
	} else return 1;

	return 0;
}

/* Manual entry for gurus. */
int manualdriver(char *driver, char *driveroptions)
{
	char *values[] = { NULL, NULL };	/* pointers for the values. */
	struct newtWinEntry entries[] =
		{ { "", &values[0], 0,}, { NULL, NULL, 0 } };
	int rc;
	char commandstring[STRING_SIZE];
	char *driverend;

	strcpy(driver, "");
	strcpy(driveroptions, "");
	
	rc = newtWinEntries(ctr[TR_SELECT_NETWORK_DRIVER], 
		ctr[TR_MODULE_PARAMETERS], 50, 5, 5, 40, entries, 
		ctr[TR_OK], ctr[TR_CANCEL], NULL);	
	if (rc == 0 || rc == 1)
	{
		if (strlen(values[0]))
		{
			sprintf(commandstring, "/sbin/modprobe %s", values[0]);
			if (runcommandwithstatus(commandstring, ctr[TR_LOADING_MODULE]) == 0)
			{
				if ((driverend = strchr(values[0], ' ')))
				{
					*driverend = '\0';
					strcpy(driver, values[0]);
					strcpy(driveroptions, driverend + 1);
				}				
				else
				{
					strcpy(driver, values[0]);
					strcpy(driveroptions, "");
				}
			}
			else
				errorbox(ctr[TR_UNABLE_TO_LOAD_DRIVER_MODULE]);
		}
		else
			errorbox(ctr[TR_MODULE_NAME_CANNOT_BE_BLANK]);
	}
	free(values[0]);

	return 1;
}
