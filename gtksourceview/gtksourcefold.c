/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/* gtksourcefold.c
 * This file is part of GtkSourceView
 * 
 * Copyright (C) 2005 - Jeroen Zwartepoorte <jeroen.zwartepoorte@gmail.com>
 * Copyright (C) 2010 - Jose Aliste <jaliste@src.gnome.org>
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

#include "gtksourcefold.h"
#include "gtksourcefold-private.h"

G_DEFINE_BOXED_TYPE (GtkSourceFold, gtk_source_fold, gtk_source_fold_copy, gtk_source_fold_free)

GtkSourceFold *
_gtk_source_fold_new (GtkSourceBuffer   *buffer,
		      const GtkTextIter *begin,
		      const GtkTextIter *end)
{
	GtkSourceFold *fold;

	fold = g_slice_new (GtkSourceFold);

	fold->parent = NULL;
	fold->children = NULL;
	fold->last_child = NULL;
	fold->folded = FALSE;
	fold->buffer = GTK_TEXT_BUFFER (buffer);

	fold->start_mark = g_object_ref (gtk_text_buffer_create_mark (fold->buffer,
                                                                      NULL, begin, FALSE));
	fold->end_mark = g_object_ref (gtk_text_buffer_create_mark (fold->buffer,
                                                                    NULL, end, TRUE));
	return fold;
}

/**
 * gtk_source_fold_free:
 * @fold: a #GtkSourceFold.
 *
 * Free a fold that was created using gtk_source_fold_copy. Useful for language
 * bindings.
 **/
void
gtk_source_fold_free (GtkSourceFold *fold)
{
	g_return_if_fail (fold != NULL);

	if (!gtk_text_mark_get_deleted (fold->start_mark))
	{
		gtk_text_buffer_delete_mark (fold->buffer, fold->start_mark);
	}
	g_object_unref (fold->start_mark);

	if (!gtk_text_mark_get_deleted (fold->end_mark))
	{
		gtk_text_buffer_delete_mark (fold->buffer, fold->end_mark);
	}
	g_object_unref (fold->end_mark);

	if (fold->children)
	{
		g_list_free (fold->children);
	}

	g_free (fold);
}

/**
 * gtk_source_fold_copy:
 * @fold: a #GtkSourceFold.
 *
 * Copy the specified fold. Useful for language bindings.
 *
 * Return value: a copy of the specified #GtkSourceFold.
 **/
GtkSourceFold *
gtk_source_fold_copy (const GtkSourceFold *fold)
{
	GtkSourceFold *copy;

	g_return_val_if_fail (fold != NULL, NULL);

	copy = g_slice_new (GtkSourceFold);
	*copy = *fold;
	copy->children = g_list_copy (fold->children);
	g_object_ref (copy->start_mark);
	g_object_ref (copy->end_mark);

	return copy;
}

/**
 * gtk_source_fold_get_folded:
 * @fold: a #GtkSourceFold.
 *
 * Return value: TRUE if the fold is currently collapsed. FALSE if it is
 * expanded.
 **/
gboolean
gtk_source_fold_get_folded (GtkSourceFold *fold)
{
	g_return_val_if_fail (fold != NULL, FALSE);

	return fold->folded;
}

/**
 * gtk_source_fold_set_folded:
 * @fold: a #GtkSourceFold.
 * @folded: a gboolean.
 *
 * Collapse the fold when folded is TRUE. Expand the fold otherwise.
 **/
void
gtk_source_fold_set_folded (GtkSourceFold *fold,
			    gboolean       folded)
{
	g_return_if_fail (fold != NULL);

	folded = (folded != FALSE);

	if (fold->folded == folded)
	{
		return;
	}
	fold->folded = folded;
}

/**
 * gtk_source_fold_get_bounds:
 * @fold: a #GtkSourceFold.
 * @begin: a #GtkTextIter.
 * @end: a #GtkTextIter.
 *
 * Returns the bounds of the fold (begin, end) using the provided #GtkTextIters.
 **/
void
gtk_source_fold_get_bounds (GtkSourceFold *fold,
			    GtkTextIter   *begin,
			    GtkTextIter   *end)
{
	g_return_if_fail (fold != NULL);

	/* FIXME: the following check comes from old code. 
	 * maybe it does not make sanse anymore*/
	if (gtk_text_mark_get_deleted (fold->start_mark))
		g_message ("starting mark DELETED!");

	if (begin != NULL) 
	{
		gtk_text_buffer_get_iter_at_mark (fold->buffer, begin, fold->start_mark);
	}
	if (end != NULL)
	{
		gtk_text_buffer_get_iter_at_mark (fold->buffer, end, fold->end_mark);
	}
}

/**
 * gtk_source_fold_get_buffer:
 * @fold: a #GtkSourceFold.
 *
 * Return value: the #GtkSourceBuffer that this fold is part of.
 **/
GtkSourceBuffer *
gtk_source_fold_get_buffer (GtkSourceFold *fold)
{
	g_return_val_if_fail (fold != NULL, NULL);

	return GTK_SOURCE_BUFFER (fold->buffer);
}

/**
 * gtk_source_fold_get_parent:
 * @fold: a #GtkSourceFold.
 *
 * Return value: the parent #GtkSourceFold, or NULL if this is a root fold.
 **/
GtkSourceFold *
gtk_source_fold_get_parent (GtkSourceFold *fold)
{
	g_return_val_if_fail (fold != NULL, NULL);

	return fold->parent;
}

/**
 * gtk_source_fold_get_children:
 * @fold: a #GtkSourceFold.
 *
 * Return value: the list of fold children, sorted by appearance.
 **/
const GList *
gtk_source_fold_get_children (GtkSourceFold *fold)
{
	g_return_val_if_fail (fold != NULL, NULL);

	return fold->children;
}

void
gtk_source_fold_get_lines (GtkSourceFold *fold,
			   gint *start_line,
			   gint *end_line)
{
	GtkTextIter iter_start, iter_stop;

	g_return_if_fail (fold != NULL);
	
	gtk_source_fold_get_bounds (fold, &iter_start, &iter_stop);

	/* The end iter of the fold is on the next line, so if the end
	 * iter is at the start of the line, go back a line. */
	if (gtk_text_iter_starts_line (&iter_stop))
	{
		gtk_text_iter_backward_line (&iter_stop);
	}
	if (start_line)
	{
		*start_line = gtk_text_iter_get_line (&iter_start);
	}
	if (end_line)
	{
		*end_line = gtk_text_iter_get_line (&iter_stop);
	}
}
