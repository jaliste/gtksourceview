/*
 *  gtksourcefold-private.h
 *
 *  Copyright (C) 2005 - Jeroen Zwartepoorte <jeroen.zwartepoorte@gmail.com>
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

#ifndef __GTK_SOURCE_FOLD_PRIVATE_H__
#define __GTK_SOURCE_FOLD_PRIVATE_H__

#include "gtksourcefold.h"

G_BEGIN_DECLS

struct _GtkSourceFold
{
	/* Markers for the start & end of the fold. */
	GtkTextMark	*start_mark;
	GtkTextMark	*end_mark;

	GSequenceIter	*node;
	GtkTextBuffer	*buffer;
	GtkSourceFold	*parent;

	/* TRUE if the fold is collapsed. */
	gint		 folded : 1;

};

GtkSourceFold	*_gtk_source_fold_new	(GtkSourceBuffer	*buffer,
					 const GtkTextIter	*begin,
					 const GtkTextIter	*end);

G_END_DECLS

#endif  /* __GTK_SOURCE_FOLD_PRIVATE_H__ */
