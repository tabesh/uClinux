/* 
 * $Id: ucontact.c,v 1.32.2.4 2005/07/27 12:40:31 janakj Exp $ 
 *
 * Usrloc contact structure
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * ---------
 * 2003-03-12 added replication mark and three zombie states (nils)
 * 2004-03-17 generic callbacks added (bogdan)
 * 2004-06-07 updated to the new DB api (andrei)
 */



#include "ucontact.h"
#include <string.h>             /* memcpy */
#include "../../mem/shm_mem.h"
#include "../../ut.h"
#include "../../dprint.h"
#include "../../db/db.h"
#include "ul_mod.h"
#include "ul_callback.h"


/*
 * Create a new contact structure
 */
int new_ucontact(str* _dom, str* _aor, str* _contact, time_t _e, qvalue_t _q,
		 str* _callid, int _cseq, unsigned int _flags, ucontact_t** _c, str* _ua, str* _recv)
{
	*_c = (ucontact_t*)shm_malloc(sizeof(ucontact_t));
	if (!(*_c)) {
	        LOG(L_ERR, "new_ucontact(): No memory left\n");
		return -1;
	}
	memset(*_c, 0, sizeof(ucontact_t));

	(*_c)->domain = _dom;
	(*_c)->aor = _aor;

	(*_c)->c.s = (char*)shm_malloc(_contact->len);
	if ((*_c)->c.s == 0) {
		LOG(L_ERR, "new_ucontact(): No memory left 2\n");
		goto error;
	}
	memcpy((*_c)->c.s, _contact->s, _contact->len);
	(*_c)->c.len = _contact->len;

	(*_c)->expires = _e;
	(*_c)->q = _q;

	(*_c)->callid.s = (char*)shm_malloc(_callid->len);
	if ((*_c)->callid.s == 0) {
		LOG(L_ERR, "new_ucontact(): No memory left 4\n");
		goto error;
	}
	memcpy((*_c)->callid.s, _callid->s, _callid->len);
	(*_c)->callid.len = _callid->len;

	(*_c)->user_agent.s = (char*)shm_malloc(_ua->len);
	if ((*_c)->user_agent.s == 0) {
		LOG(L_ERR, "new_ucontact(): No memory left 8\n");
		goto error;
	}
	memcpy((*_c)->user_agent.s, _ua->s, _ua->len);
	(*_c)->user_agent.len = _ua->len;

	if (_recv) {
		(*_c)->received.s = (char*)shm_malloc(_recv->len);
		if ((*_c)->received.s == 0) {
			LOG(L_ERR, "new_ucontact(): No memory left\n");
			goto error;
		}
		memcpy((*_c)->received.s, _recv->s, _recv->len);
		(*_c)->received.len = _recv->len;
	} else {
		(*_c)->received.s = 0;
		(*_c)->received.len = 0;
	}

	(*_c)->cseq = _cseq;
	(*_c)->state = CS_NEW;
	(*_c)->flags = _flags;
        return 0;

 error:
	if (*_c) {
		if ((*_c)->received.s) shm_free((*_c)->received.s);
		if ((*_c)->user_agent.s) shm_free((*_c)->user_agent.s);
		if ((*_c)->callid.s) shm_free((*_c)->callid.s);
		if ((*_c)->c.s) shm_free((*_c)->c.s);
		shm_free(*_c);
	}
	return -1;
}	


/*
 * Free all memory associated with given contact structure
 */
void free_ucontact(ucontact_t* _c)
{
	if (!_c) return;
	if (_c->received.s) shm_free(_c->received.s);
	if (_c->user_agent.s) shm_free(_c->user_agent.s);
	if (_c->callid.s) shm_free(_c->callid.s);
	if (_c->c.s) shm_free(_c->c.s);
	shm_free(_c);
}


/*
 * Print contact, for debugging purposes only
 */
void print_ucontact(FILE* _f, ucontact_t* _c)
{
	time_t t = time(0);
	char* st;

	switch(_c->state) {
	case CS_NEW:   st = "CS_NEW";     break;
	case CS_SYNC:  st = "CS_SYNC";    break;
	case CS_DIRTY: st = "CS_DIRTY";   break;
	default:       st = "CS_UNKNOWN"; break;
	}

	fprintf(_f, "~~~Contact(%p)~~~\n", _c);
	fprintf(_f, "domain    : '%.*s'\n", _c->domain->len, ZSW(_c->domain->s));
	fprintf(_f, "aor       : '%.*s'\n", _c->aor->len, ZSW(_c->aor->s));
	fprintf(_f, "Contact   : '%.*s'\n", _c->c.len, ZSW(_c->c.s));
	fprintf(_f, "Expires   : ");
	if (_c->flags & FL_PERMANENT) {
		fprintf(_f, "Permanent\n");
	} else {
		if (_c->expires == 0) {
			fprintf(_f, "Deleted\n");
		} else if (t > _c->expires) {
			fprintf(_f, "Expired\n");
		} else {
			fprintf(_f, "%u\n", (unsigned int)(_c->expires - t));
		}
	}
	fprintf(_f, "q         : %s\n", q2str(_c->q, 0));
	fprintf(_f, "Call-ID   : '%.*s'\n", _c->callid.len, ZSW(_c->callid.s));
	fprintf(_f, "CSeq      : %d\n", _c->cseq);
	fprintf(_f, "User-Agent: '%.*s'\n", _c->user_agent.len, ZSW(_c->user_agent.s));
	fprintf(_f, "received  : '%.*s'\n", _c->received.len, ZSW(_c->received.s));
	fprintf(_f, "State     : %s\n", st);
	fprintf(_f, "Flags     : %u\n", _c->flags);
	fprintf(_f, "next      : %p\n", _c->next);
	fprintf(_f, "prev      : %p\n", _c->prev);
	fprintf(_f, "~~~/Contact~~~~\n");
}


/*
 * Update ucontact structure in memory
 */
int mem_update_ucontact(ucontact_t* _c, time_t _e, qvalue_t _q, str* _cid, int _cs,
			unsigned int _set, unsigned int _res, str* _ua, str* _recv)
{
	char* ptr;
	
	if (_c->callid.len < _cid->len) {
		ptr = (char*)shm_malloc(_cid->len);
		if (ptr == 0) {
			LOG(L_ERR, "update_ucontact(): No memory left\n");
			return -1;
		}
		
		memcpy(ptr, _cid->s, _cid->len);
		shm_free(_c->callid.s);
		_c->callid.s = ptr;
	} else {
		memcpy(_c->callid.s, _cid->s, _cid->len);
	}
	_c->callid.len = _cid->len;

	if (_c->user_agent.len < _ua->len) {
		ptr = (char*)shm_malloc(_ua->len);
		if (ptr == 0) {
			LOG(L_ERR, "update_ucontact(): No memory left\n");
			return -1;
		}

		memcpy(ptr, _ua->s, _ua->len);
		shm_free(_c->user_agent.s);
		_c->user_agent.s = ptr;
	} else {
		memcpy(_c->user_agent.s, _ua->s, _ua->len);
	}
	_c->user_agent.len = _ua->len;

	if (_recv) {
		if (_c->received.len < _recv->len) {
			ptr = (char*)shm_malloc(_recv->len);
			if (ptr == 0) {
				LOG(L_ERR, "update_ucontact(): No memory left\n");
				return -1;
			}
			memcpy(ptr, _recv->s, _recv->len);
			/* previous received might be NULL! -bogdan */
			if (_c->received.s)
				shm_free(_c->received.s);
			_c->received.s = ptr;
		} else {
			memcpy(_c->received.s, _recv->s, _recv->len);
		}
		_c->received.len = _recv->len;
	} else {
		if (_c->received.s) shm_free(_c->received.s);
		_c->received.s = 0;
		_c->received.len = 0;
	}

	_c->expires = _e;
	_c->q = _q;
	_c->cseq = _cs;
	_c->flags |= _set;
	_c->flags &= ~_res;

	return 0;
}


/* ================ State related functions =============== */


/*
 * Update state of the contact
 */
void st_update_ucontact(ucontact_t* _c)
{
	switch(_c->state) {
	case CS_NEW:
		     /* Contact is new and is not in the database yet,
		      * we remain in the same state here because the
		      * contact must be inserted later in the timer
		      */
		break;

	case CS_SYNC:
		     /* For db mode 2 a modified contact needs to be 
			  * updated also in the database, so transit into 
			  * CS_DIRTY and let the timer to do the update 
			  * again. For db mode 1 the db update is already
			  * done and we don't have to change the state.
		      */
		if (db_mode == WRITE_BACK) {
			_c->state = CS_DIRTY;
		}
		break;

	case CS_DIRTY:
		     /* Modification of dirty contact results in
		      * dirty contact again, don't change anything
		      */
		break;
	}
}


/*
 * Update state of the contact
 * Returns 1 if the contact should be
 * delete from memory immediately,
 * 0 otherwise
 */
int st_delete_ucontact(ucontact_t* _c)
{
	switch(_c->state) {
	case CS_NEW:
		     /* Contact is new and isn't in the database
		      * yet, we can delete it from the memory
		      * safely.
		      */
		return 1;

	case CS_SYNC:
	case CS_DIRTY:
		     /* Contact is in the database,
		      * we cannot remove it from the memory 
		      * directly, but we can set expires to zero
		      * and the timer will take care of deleting 
		      * the contact from the memory as well as 
		      * from the database
		      */
		if (db_mode == WRITE_BACK) {
			     /* Reset permanent flag */
			_c->flags &= ~FL_PERMANENT;
			_c->expires = 0;
			return 0;
		} else {
			     /* WRITE_THROUGH or NO_DB -- we can
			      * remove it from memory immediately and
			      * the calling function would also remove
			      * it from the database if needed
			      */
			return 1;
		}
	}

	return 0; /* Makes gcc happy */
}


/*
 * Called when the timer is about to delete
 * an expired contact, this routine returns
 * 1 if the contact should be removed from
 * the database and 0 otherwise
 */
int st_expired_ucontact(ucontact_t* _c)
{
	     /* There is no need to change contact
	      * state, because the contact will
	      * be deleted anyway
	      */

	switch(_c->state) {
	case CS_NEW:
		     /* Contact is not in the database
		      * yet, remove it from memory only
		      */
		return 0;

	case CS_SYNC:
	case CS_DIRTY:
		     /* Remove from database here */
		return 1;
	}

	return 0; /* Makes gcc happy */
}


/*
 * Called when the timer is about flushing the contact,
 * updates contact state and returns 1 if the contact
 * should be inserted, 2 if update , 3 if delete 
 * from memory, 4 if delete from database and 0 otherwise
 */
int st_flush_ucontact(ucontact_t* _c)
{
	switch(_c->state) {
	case CS_NEW:
		     /* Contact is new and is not in
		      * the database yet so we have
		      * to insert it
		      */
		_c->state = CS_SYNC;
		return 1;

	case CS_SYNC:
		     /* Contact is synchronized, do
		      * nothing
		      */
		return 0;

	case CS_DIRTY:
		     /* Contact has been modified and
		      * is in the db already so we
		      * have to update it
		      */
		_c->state = CS_SYNC;
		return 2;
	}

	return 0; /* Makes gcc happy */
}


/* ============== Database related functions ================ */


/*
 * Insert contact into the database
 */
int db_insert_ucontact(ucontact_t* _c)
{
	char b[256];
	char* dom;
	db_key_t keys[10];
	db_val_t vals[10];
	
	if (_c->flags & FL_MEM) {
		return 0;
	}

	keys[0] = user_col.s;
	keys[1] = contact_col.s;
	keys[2] = expires_col.s;
	keys[3] = q_col.s;
	keys[4] = callid_col.s;
	keys[5] = cseq_col.s;
	keys[6] = flags_col.s;
	keys[7] = user_agent_col.s;
	keys[8] = received_col.s;
	keys[9] = domain_col.s;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = _c->aor->s;
	vals[0].val.str_val.len = _c->aor->len;

	vals[1].type = DB_STR;
	vals[1].nul = 0;
	vals[1].val.str_val.s = _c->c.s; 
	vals[1].val.str_val.len = _c->c.len;

	vals[2].type = DB_DATETIME;
	vals[2].nul = 0;
	vals[2].val.time_val = _c->expires;

	vals[3].type = DB_DOUBLE;
	vals[3].nul = 0;
	vals[3].val.double_val = q2double(_c->q);

	vals[4].type = DB_STR;
	vals[4].nul = 0;
	vals[4].val.str_val.s = _c->callid.s;
	vals[4].val.str_val.len = _c->callid.len;

	vals[5].type = DB_INT;
	vals[5].nul = 0;
	vals[5].val.int_val = _c->cseq;

	vals[6].type = DB_INT;
	vals[6].nul = 0;
	vals[6].val.bitmap_val = _c->flags;

	vals[7].type = DB_STR;
	vals[7].nul = 0;
	vals[7].val.str_val.s = _c->user_agent.s;
	vals[7].val.str_val.len = _c->user_agent.len;

	vals[8].type = DB_STR;

	if (_c->received.s == 0) {
		vals[8].nul = 1;
	} else {
		vals[8].nul = 0;
		vals[8].val.str_val.s = _c->received.s;
		vals[8].val.str_val.len = _c->received.len;
	}

	if (use_domain) {
		dom = q_memchr(_c->aor->s, '@', _c->aor->len);
		vals[0].val.str_val.len = dom - _c->aor->s;

		vals[9].type = DB_STR;
		vals[9].nul = 0;
		vals[9].val.str_val.s = dom + 1;
		vals[9].val.str_val.len = _c->aor->s + _c->aor->len - dom - 1;
	}

	     /* FIXME */
	memcpy(b, _c->domain->s, _c->domain->len);
	b[_c->domain->len] = '\0';
	if (ul_dbf.use_table(ul_dbh, b) < 0) {
		LOG(L_ERR, "db_insert_ucontact(): Error in use_table\n");
		return -1;
	}

	if (ul_dbf.insert(ul_dbh, keys, vals, (use_domain) ? 10 : 9) < 0) {
		LOG(L_ERR, "db_insert_ucontact(): Error while inserting contact\n");
		return -1;
	}

	return 0;
}


/*
 * Update contact in the database
 */
int db_update_ucontact(ucontact_t* _c)
{
	char b[256];
	char* dom;
	db_key_t keys1[3];
	db_val_t vals1[3];

	db_key_t keys2[7];
	db_val_t vals2[7];

	if (_c->flags & FL_MEM) {
		return 0;
	}

	keys1[0] = user_col.s;
	keys1[1] = contact_col.s;
	keys1[2] = domain_col.s;
	keys2[0] = expires_col.s;
	keys2[1] = q_col.s;
	keys2[2] = callid_col.s;
	keys2[3] = cseq_col.s;
	keys2[4] = flags_col.s;
	keys2[5] = user_agent_col.s;
	keys2[6] = received_col.s;
	
	vals1[0].type = DB_STR;
	vals1[0].nul = 0;
	vals1[0].val.str_val = *_c->aor;

	vals1[1].type = DB_STR;
	vals1[1].nul = 0;
	vals1[1].val.str_val = _c->c;

	vals2[0].type = DB_DATETIME;
	vals2[0].nul = 0;
	vals2[0].val.time_val = _c->expires;

	vals2[1].type = DB_DOUBLE;
	vals2[1].nul = 0;
	vals2[1].val.double_val = q2double(_c->q);

	vals2[2].type = DB_STR;
	vals2[2].nul = 0;
	vals2[2].val.str_val = _c->callid;

	vals2[3].type = DB_INT;
	vals2[3].nul = 0;
	vals2[3].val.int_val = _c->cseq;

	vals2[4].type = DB_INT;
	vals2[4].nul = 0;
	vals2[4].val.bitmap_val = _c->flags;

	vals2[5].type = DB_STR;
	vals2[5].nul = 0;
	vals2[5].val.str_val = _c->user_agent;

	vals2[6].type = DB_STR;
	if (_c->received.s == 0) {
		vals2[6].nul = 1;
	} else {
		vals2[6].nul = 0;
		vals2[6].val.str_val = _c->received;
	}

	if (use_domain) {
		dom = q_memchr(_c->aor->s, '@', _c->aor->len);
		vals1[0].val.str_val.len = dom - _c->aor->s;

		vals1[2].type = DB_STR;
		vals1[2].nul = 0;
		vals1[2].val.str_val.s = dom + 1;
		vals1[2].val.str_val.len = _c->aor->s + _c->aor->len - dom - 1;
	}

	     /* FIXME */
	memcpy(b, _c->domain->s, _c->domain->len);
	b[_c->domain->len] = '\0';
	if (ul_dbf.use_table(ul_dbh, b) < 0) {
		LOG(L_ERR, "db_upd_ucontact(): Error in use_table\n");
		return -1;
	}

	if (ul_dbf.update(ul_dbh, keys1, 0, vals1, keys2, vals2, (use_domain) ? 3 : 2, 7) < 0) {
		LOG(L_ERR, "db_upd_ucontact(): Error while updating database\n");
		return -1;
	}

	return 0;
}


/*
 * Delete contact from the database
 */
int db_delete_ucontact(ucontact_t* _c)
{
	char b[256];
	char* dom;
	db_key_t keys[3];
	db_val_t vals[3];

	if (_c->flags & FL_MEM) {
		return 0;
	}

	keys[0] = user_col.s;
	keys[1] = contact_col.s;
	keys[2] = domain_col.s;

	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val = *_c->aor;

	vals[1].type = DB_STR;
	vals[1].nul = 0;
	vals[1].val.str_val = _c->c;

	if (use_domain) {
		dom = q_memchr(_c->aor->s, '@', _c->aor->len);
		vals[0].val.str_val.len = dom - _c->aor->s;

		vals[2].type = DB_STR;
		vals[2].nul = 0;
		vals[2].val.str_val.s = dom + 1;
		vals[2].val.str_val.len = _c->aor->s + _c->aor->len - dom - 1;
	}

	     /* FIXME */
	memcpy(b, _c->domain->s, _c->domain->len);
	b[_c->domain->len] = '\0';
	if (ul_dbf.use_table(ul_dbh, b) < 0) {
		LOG(L_ERR, "db_del_ucontact: Error in use_table\n");
		return -1;
	}

	if (ul_dbf.delete(ul_dbh, keys, 0, vals, (use_domain) ? (3) : (2)) < 0) {
		LOG(L_ERR, "db_del_ucontact(): Error while deleting from database\n");
		return -1;
	}

	return 0;
}


/*
 * Update ucontact with new values
 */
int update_ucontact(ucontact_t* _c, time_t _e, qvalue_t _q, str* _cid, int _cs,
		    unsigned int _set, unsigned int _res, str* _ua, str* _recv)
{
	/* run callbacks for UPDATE event */
	if (exists_ulcb_type(UL_CONTACT_UPDATE)) {
		run_ul_callbacks( UL_CONTACT_UPDATE, _c);
	}

	/* we have to update memory in any case, but database directly
	 * only in db_mode 1 */
	if (mem_update_ucontact(_c, _e, _q, _cid, _cs, _set, _res, _ua, _recv) < 0) {
		LOG(L_ERR, "update_ucontact(): Error while updating\n");
		return -1;
	}
	st_update_ucontact(_c);
	if (db_mode == WRITE_THROUGH) {
		if (db_update_ucontact(_c) < 0) {
			LOG(L_ERR, "update_ucontact(): Error while updating database\n");
		}
	}
	return 0;
}
