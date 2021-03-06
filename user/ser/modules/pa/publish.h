/*
 * Presence Agent, publish handling
 *
 * $Id: publish.h,v 1.3 2004/08/24 09:00:33 janakj Exp $
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
 */

#ifndef PUBLISH_H
#define PUBLISH_H

#include "../../parser/msg_parser.h"

/*
 * Handle a publish Request
 */
int handle_publish(struct sip_msg* _m, char* _domain, char* _s2);

/*
 * FIFO function for publishing events
 */
int fifo_pa_publish(FILE *stream, char *response_file);
/*
 * FIFO function for publishing presence
 */
int fifo_pa_presence(FILE *stream, char *response_file);
/*
 * FIFO function for publishing location
 */
int fifo_pa_location(FILE *stream, char *response_file);

/*
 * FIFO function for publishing location for a contact
 *
 * :pa_location_contact:
 * pdomain (registrar or jabber)
 * presentity_uri
 * presentity_contact
 * presentity_location (civil or geopriv)
 * priority
 * expires
 *
 */
int fifo_pa_location_contact(FILE *fifo, char *response_file);

/*
 * FIFO function to cause ser to reload watcherinfo for a presentity
 */
int fifo_pa_watcherinfo(FILE *fifo, char *response_file);

#endif /* PUBLISH_H */
