/* SmoothWall setup program.
 *
 * This program is distributed under the terms of the GNU General Public
 * Licence.  See the file COPYING for details.
 *
 * (c) Lawrence Manning, 2001
 * Stuff for setting the hostname.
 * 
 * $Id: hostname.c,v 1.6.2.1 2004/04/14 22:05:41 gespinasse Exp $
 * 
 */
 
#include "setup.h"
 
extern FILE *flog;
extern char *mylog;

extern char **ctr;

extern int automode;

int handlehostname(void)
{
	char hostname[STRING_SIZE] = "";
	struct keyvalue *kv = initkeyvalues();
	char *values[] = { hostname, NULL };	/* pointers for the values. */
	struct newtWinEntry entries[] =
		{ { "", &values[0], 0,}, { NULL, NULL, 0 } };
	int rc;
	int result;
	
	if (!(readkeyvalues(kv, CONFIG_ROOT "/main/settings")))
	{
		freekeyvalues(kv);
		errorbox(ctr[TR_UNABLE_TO_OPEN_SETTINGS_FILE]);
		return 0;
	}	
	
	strcpy(hostname, SNAME);
	findkey(kv, "HOSTNAME", hostname);
	
	for (;;)
	{
		rc = newtWinEntries(ctr[TR_HOSTNAME], ctr[TR_ENTER_HOSTNAME],
			50, 5, 5, 40, entries, ctr[TR_OK], ctr[TR_CANCEL], NULL);
		
		if (rc == 1)
		{
			strcpy(hostname, values[0]);
			if (!(strlen(hostname)))
				errorbox(ctr[TR_HOSTNAME_CANNOT_BE_EMPTY]);
			else if (strchr(hostname, ' '))
				errorbox(ctr[TR_HOSTNAME_CANNOT_CONTAIN_SPACES]);
			else if (strlen(hostname) != strspn(hostname,
				"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-"))
				errorbox(ctr[TR_HOSTNAME_NOT_VALID_CHARS]);
			else
			{
				replacekeyvalue(kv, "HOSTNAME", hostname);
				writekeyvalues(kv, CONFIG_ROOT "/main/settings");
				writehostsfiles();
				result = 1;
				break;
			}
		}
		else
		{
			result = 0;
			break;
		}
	}
	free(values[0]);
	freekeyvalues(kv);
	
	return result;
}	
