/*
 *   IRC - Internet Relay Chat, src/modules/m_mkpasswd.c
 *   (C) 2001 The UnrealIRCd Team
 *
 *   mkpasswd command
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(m_mkpasswd);

#define MSG_MKPASSWD 	"MKPASSWD"	

ModuleHeader MOD_HEADER(m_mkpasswd)
  = {
	"m_mkpasswd",
	"4.2",
	"command /mkpasswd", 
	"3.2-b8-1",
	NULL 
    };

MOD_INIT(m_mkpasswd)
{
	CommandAdd(modinfo->handle, MSG_MKPASSWD, m_mkpasswd, MAXPARA, M_USER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD(m_mkpasswd)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(m_mkpasswd)
{
	return MOD_SUCCESS;
}

/*
** m_mkpasswd
**      parv[1] = password to encrypt
*/
CMD_FUNC(m_mkpasswd)
{
	short	type;
	char	*result = NULL;

	if (!MKPASSWD_FOR_EVERYONE && !IsOper(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, sptr->name);
		return -1;
	}
	if (!IsOper(sptr))
	{
		/* Non-opers /mkpasswd usage: lag them up, and send a notice to eyes snomask.
		 * This notice is always sent, even in case of bad usage/bad auth methods/etc.
		 */
		sptr->local->since += 7;
		sendto_snomask(SNO_EYES, "*** /mkpasswd used by %s (%s@%s)",
			sptr->name, sptr->user->username, GetHost(sptr));
	}

	if ((parc < 3) || BadPtr(parv[2]))
	{
		sendto_one(sptr, ":%s NOTICE %s :*** Syntax: /mkpasswd <authmethod> :parameter",
			me.name, sptr->name);
		return 0;
	}
	/* Don't want to take any risk ;p. -- Syzop */
	if (strlen(parv[2]) > 64)
	{
		sendto_one(sptr, ":%s NOTICE %s :*** Your parameter (text-to-hash) is too long.",
			me.name, sptr->name);
		return 0;
	}
	if ((type = Auth_FindType(NULL, parv[1])) == -1)
	{
		sendto_one(sptr, 
			":%s NOTICE %s :*** %s is not an enabled authentication method",
				me.name, sptr->name, parv[1]);
		return 0;
	}

	if ((type == AUTHTYPE_MD5) || (type == AUTHTYPE_SHA1) ||
	    (type == AUTHTYPE_RIPEMD160) || (type == AUTHTYPE_UNIXCRYPT))
	{
		sendnotice(sptr, "ERROR: Deprecated authentication type '%s'. Use 'argon2' instead. "
		           "See https://www.unrealircd.org/docs/Authentication_types for the complete list.",
		           parv[1]);
		return 0;
	}

	if (!(result = Auth_Make(type, parv[2])))
	{
		sendto_one(sptr, 
			":%s NOTICE %s :*** Authentication method %s failed",
				me.name, sptr->name, parv[1]);
		return 0;
	}

	sendnotice(sptr, "*** Authentication phrase (method=%s, para=%s) is: %s",
		parv[1], parv[2], result);

	return 0;
}
