/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcehighlighter.h
 *
 *  Copyright (C) 2003 - Gustavo Giráldez
 *  Copyright (C) 2005 - Marco Barisione, Emanuele Aina
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

#ifndef __GTK_HIGHLIGHT_ENGINE_H__
#define __GTK_HIGHLIGHT_ENGINE_H__

#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceengine.h>
#include <gtksourceview/gtksourcelanguage.h>

G_BEGIN_DECLS

#define GTK_TYPE_HIGHLIGHT_ENGINE            (_gtk_highlight_engine_get_type ())
#define GTK_HIGHLIGHT_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_HIGHLIGHT_ENGINE, GtkHighlightEngine))
#define GTK_HIGHLIGHT_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_HIGHLIGHT_ENGINE, GtkHighlightEngineClass))
#define GTK_IS_HIGHLIGHT_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_HIGHLIGHT_ENGINE))
#define GTK_IS_HIGHLIGHT_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HIGHLIGHT_ENGINE))
#define GTK_HIGHLIGHT_ENGINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_HIGHLIGHT_ENGINE, GtkHighlightEngineClass))

typedef struct _GtkHighlightEngine        GtkHighlightEngine;
typedef struct _GtkHighlightEngineClass   GtkHighlightEngineClass;
typedef struct _GtkHighlightEnginePrivate GtkHighlightEnginePrivate;

struct _GtkHighlightEngine
{
	GObject parent_instance;

	/*< private >*/
	GtkHighlightEnginePrivate *priv;
};

struct _GtkHighlightEngineClass
{
	GObjectClass parent_class;
};

GType		 	_gtk_highlight_engine_get_type	(void) G_GNUC_CONST;

GtkHighlightEngine * 	_gtk_highlight_engine_new (void);
void 			_gtk_highlight_engine_set_style_scheme	(GtkHighlightEngine     *engine,
				       				 GtkSourceStyleScheme 	*scheme);
void 			_gtk_highlight_engine_set_styles_map	(GtkHighlightEngine     *engine,
				       			         GHashTable 		*styles);
void			_gtk_highlight_engine_set_analyzer     	(GtkHighlightEngine	*engine,
								 GtkSourceEngine	*se);

void			_gtk_highlight_engine_attach_buffer	(GtkHighlightEngine 	*engine,
				  			     	 GtkTextBuffer     	*buffer);
G_END_DECLS

#endif /* __GTK_HIGHLIGHT_ENGINE_H__ */
