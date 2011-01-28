/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcebuffer-private.h
 *
 *  Copyright (C) 2008 - Paolo Maggi, Paolo Borelli
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_SOURCE_BUFFER_PRIVATE_H__
#define __GTK_SOURCE_BUFFER_PRIVATE_H__

#include <gtksourceview/gtksourcebuffer.h>

G_BEGIN_DECLS

/* fold methods. */
GtkSourceFold * 	gtk_source_buffer_add_fold		(GtkSourceBuffer        *buffer,
								 const GtkTextIter      *begin,
								 const GtkTextIter      *end);
void			gtk_source_buffer_remove_fold		(GtkSourceBuffer        *buffer,
								 GtkSourceFold          *fold);
void			gtk_source_buffer_remove_folds_in_region
 								(GtkSourceBuffer        *buffer,
								 const GtkTextIter      *begin,
								 const GtkTextIter      *end);
GList *			gtk_source_buffer_get_folds_in_region	(GtkSourceBuffer        *buffer,
								 const GtkTextIter      *begin,
								 const GtkTextIter      *end);
gboolean		gtk_source_buffer_get_folds_enabled	(GtkSourceBuffer        *buffer);
void			gtk_source_buffer_set_folds_enabled	(GtkSourceBuffer        *buffer,
								 gboolean                folds_enabled);
void			gtk_source_buffer_fold_expand		(GtkSourceBuffer        *buffer, 
								 GtkSourceFold          *fold);
void 			gtk_source_buffer_fold_collapse		(GtkSourceBuffer        *buffer,
								 GtkSourceFold          *fold);
G_END_DECLS

#endif /* __GTK_SOURCE_BUFFER_PRIVATE_H__ */
