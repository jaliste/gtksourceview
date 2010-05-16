/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtkhighlightengine.h
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

#ifndef __GTK_SOURCE_SYNTAX_HIGHLIGHTER_H__
#define __GTK_SOURCE_SYNTAX_HIGHLIGHTER_H__

#include <gtksourceview/gtksourceengine.h>

G_BEGIN_DECLS

#define GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER            (_gtk_source_syntax_highlighter_get_type ())
#define GTK_SOURCE_SYNTAX_HIGHLIGHTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER, GtkSourceSyntaxHighlighter))
#define GTK_SOURCE_SYNTAX_HIGHLIGHTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER, GtkSourceSyntaxHighlighterClass))
#define GTK_IS_SOURCE_SYNTAX_HIGHLIGHTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER))
#define GTK_IS_SOURCE_SYNTAX_HIGHLIGHTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER))
#define GTK_SOURCE_SYNTAX_HIGHLIGHTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SOURCE_SYNTAX_HIGHLIGHTER, GtkSourceSyntaxHighlighterClass))

typedef struct _GtkSourceSyntaxHighlighter        GtkSourceSyntaxHighlighter;
typedef struct _GtkSourceSyntaxHighlighterClass   GtkSourceSyntaxHighlighterClass;
typedef struct _GtkSourceSyntaxHighlighterPrivate GtkSourceSyntaxHighlighterPrivate;

struct _GtkSourceSyntaxHighlighter
{
	GObject parent_instance;

	/*< private >*/
	GtkSourceSyntaxHighlighterPrivate *priv;
};

struct _GtkSourceSyntaxHighlighterClass
{
	GObjectClass parent_class;
};

GType		 	_gtk_source_syntax_highlighter_get_type	(void) G_GNUC_CONST;

GtkSourceSyntaxHighlighter * 	_gtk_source_syntax_highlighter_new (void);
void 			_gtk_source_syntax_highlighter_set_style_scheme	(GtkSourceSyntaxHighlighter     *engine,
				       				 GtkSourceStyleScheme 	*scheme);
void 			_gtk_source_syntax_highlighter_set_styles_map	(GtkSourceSyntaxHighlighter     *engine,
				       			         GHashTable 		*styles);
void			_gtk_source_syntax_highlighter_set_analyzer     	(GtkSourceSyntaxHighlighter	*engine,
								 GtkSourceEngine	*se);
G_END_DECLS

#endif /* __GTK_SOURCE_SYNTAX_HIGHLIGHTER_H__ */
