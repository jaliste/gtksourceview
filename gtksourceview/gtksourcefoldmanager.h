/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/* gtksourcefoldmanager.h
 * This file is part of GtkSourceView
 *
 * Copyright (C) 1999-2002 - Jeroen Zwartepoorte <jeroen@xs4all.nl>
 * Copyright (C) 2010 - José Aliste <jaliste@src.gnome.org>
 *
 * GtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GtkSourceView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GTK_SOURCE_FOLD_MANAGER_H__
#define __GTK_SOURCE_FOLD_MANAGER_H__

#include <glib-object.h>
#include "gtksourcefold.h"
#include "gtksourcebuffer.h"

G_BEGIN_DECLS

#define GTK_TYPE_SOURCE_FOLD_MANAGER            (gtk_source_fold_manager_get_type ())
#define GTK_SOURCE_FOLD_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManager))
#define GTK_SOURCE_FOLD_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManagerClass))
#define GTK_IS_SOURCE_FOLD_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_FOLD_MANAGER))
#define GTK_IS_SOURCE_FOLD_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SOURCE_FOLD_MANAGER))
#define GTK_SOURCE_FOLD_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManagerClass))

typedef struct _GtkSourceFoldManager        GtkSourceFoldManager;
typedef struct _GtkSourceFoldManagerClass   GtkSourceFoldManagerClass;
typedef struct _GtkSourceFoldManagerPrivate GtkSourceFoldManagerPrivate;

struct _GtkSourceFoldManager
{
	GObject parent_instance;

	/*< private >*/
	GtkSourceFoldManagerPrivate *priv;
};

struct _GtkSourceFoldManagerClass
{
	GObjectClass parent_class;
};

GType			gtk_source_fold_manager_get_type	 (void) G_GNUC_CONST;

GtkSourceFoldManager *	gtk_source_fold_manager_new 			(GtkSourceBuffer *buffer);
void			gtk_source_fold_manager_set_fold_tag 		(GtkSourceFoldManager *fold_manager, GtkTextTag *tag);

GtkSourceFold *		gtk_source_fold_manager_add_fold 		(GtkSourceFoldManager   *fold_manager,
									 const GtkTextIter *begin,
									 const GtkTextIter *end);
void			gtk_source_fold_manager_remove_fold		(GtkSourceFoldManager *fold_manager,
									 GtkSourceFold   *fold);
GtkSourceFold *		gtk_source_fold_manager_get_first_fold_intersects_region
									(GtkSourceFoldManager *manager,
									 const GtkTextIter    *begin,
									 const GtkTextIter    *end);
GList *			gtk_source_fold_manager_get_folds_in_region	(GtkSourceFoldManager *manager,
									 const GtkTextIter    *begin,
									 const GtkTextIter    *end);
void			gtk_source_fold_manager_remove_folds_in_region	(GtkSourceFoldManager *manager,
									 const GtkTextIter    *begin,
									 const GtkTextIter    *end);
GtkSourceFold *		gtk_source_fold_manager_get_fold_at_line	(GtkSourceFoldManager *manager,
									 gint                  line);
void			gtk_source_fold_manager_expand_fold		(GtkSourceFoldManager *manager, 
									 GtkSourceFold *fold);
void			gtk_source_fold_manager_collapse_fold		(GtkSourceFoldManager *manager, 
									 GtkSourceFold *fold);
G_END_DECLS

#endif /* __GTK_SOURCE_FOLD_MANAGER_H__ */
