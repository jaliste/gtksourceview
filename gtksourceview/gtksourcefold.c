/*
 *  gtksourcefold.c
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

#include "gtksourcefold.h"
#include "gtksourcefold-private.h"

GtkSourceFold *
_gtk_source_fold_new (GtkSourceBuffer   *buffer,
		      const GtkTextIter *begin,
		      const GtkTextIter *end)
{
	GtkSourceFold *fold;

	fold = g_new0 (GtkSourceFold, 1);
	fold->parent = NULL;
	fold->children = NULL;
	fold->folded = FALSE;
	fold->prelighted = FALSE;
	fold->animated = FALSE;
	fold->expander_style = GTK_EXPANDER_EXPANDED;

	fold->start_line = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer),
							NULL, begin, FALSE);
	g_object_ref (fold->start_line);
	fold->end_line = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer),
						      NULL, end, FALSE);
	g_object_ref (fold->end_line);

	return fold;
}

GType
gtk_source_fold_get_type (void)
{
	static GType our_type = 0;

	if (our_type == 0)
		our_type = g_boxed_type_register_static ("GtkSourceFold",
							 (GBoxedCopyFunc) gtk_source_fold_copy,
							 (GBoxedFreeFunc) gtk_source_fold_free);

	return our_type;
}

/**
 * gtk_source_fold_free:
 * @fold: a #GtkSourceFold.
 *
 * Free a fold that was created using gtk_source_fold_copy. Useful for language
 * bindings. Do not use otherwise.
 **/
void
gtk_source_fold_free (GtkSourceFold *fold)
{
	if (!fold)
		return;

	if (!gtk_text_mark_get_deleted (fold->start_line))
		gtk_text_buffer_delete_mark (gtk_text_mark_get_buffer (fold->start_line),
					     fold->start_line);
	g_object_unref (fold->start_line);

	if (!gtk_text_mark_get_deleted (fold->end_line))
		gtk_text_buffer_delete_mark (gtk_text_mark_get_buffer (fold->end_line),
					     fold->end_line);
	g_object_unref (fold->end_line);

	if (fold->children)
		g_list_free (fold->children);

	g_free (fold);
}

/**
 * gtk_source_fold_copy:
 * @fold: a #GtkSourceFold.
 *
 * Copy the specified fold. Useful for language bindings. Do not use otherwise.
 *
 * Return value: a copy of the specified #GtkSourceFold.
 **/
GtkSourceFold *
gtk_source_fold_copy (const GtkSourceFold *fold)
{
	GtkSourceFold *copy;

	g_return_val_if_fail (fold != NULL, NULL);

	copy = g_new (GtkSourceFold, 1);
	*copy = *fold;
	copy->children = g_list_copy (fold->children);
	g_object_ref (copy->start_line);
	g_object_ref (copy->end_line);

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

static void
reapply_fold (GtkTextBuffer *buffer,
			   GList         *folds)
{
	GtkSourceFold *fold;
	GtkTextIter begin, end;

	while (folds != NULL)
	{
		fold = folds->data;

		if (fold->folded)
		{
			gtk_text_buffer_get_iter_at_mark (buffer, &begin,
							  fold->start_line);
			gtk_text_iter_forward_to_line_end (&begin);
			gtk_text_buffer_get_iter_at_mark (buffer, &end,
							  fold->end_line);
			_gtk_source_buffer_apply_fold (GTK_SOURCE_BUFFER (buffer),
						      &begin, &end);
		}
		else if (fold->children != NULL)
		{
			reapply_fold (buffer, fold->children);
		}

		folds = g_list_next (folds);
	}
}

static void
collapse_fold (GtkTextBuffer *buffer,
	       GtkSourceFold *fold,
	       GtkTextIter   *begin,
	       GtkTextIter   *end)
{
	GtkTextIter insert;

	/* if the starting point of the fold has no text before it on the line,
	 * then only hide part of the line so the user still sees something. */
	if (gtk_text_iter_starts_line (begin))
		gtk_text_iter_forward_to_line_end (begin);

	/* hide the entire line that contains the end of the fold. */
	if (!gtk_text_iter_starts_line (end))
		gtk_text_iter_forward_line (end);

	_gtk_source_buffer_apply_fold (GTK_SOURCE_BUFFER (buffer),
				      begin, end);

	gtk_text_buffer_get_iter_at_mark (buffer, &insert,
					  gtk_text_buffer_get_insert (buffer));

	/* make the cursor visible again if it was inside the fold. */
	if (gtk_text_iter_in_range (&insert, begin, end))
	{
		if (!gtk_text_iter_forward_visible_cursor_position (&insert))
			gtk_text_iter_backward_visible_cursor_position (&insert);

		gtk_text_buffer_place_cursor (buffer, &insert);
	}

	/* if the fold collapse is animated, the style is gradually
	 * updated from a timeout handler in the view. If it isn't
	 * animated we need to set the style here. This needed when
	 * the user collapses the fold using the API instead of the GUI. */
	if (!fold->animated)
		fold->expander_style = GTK_EXPANDER_COLLAPSED;
}

static void
expand_fold (GtkTextBuffer *buffer,
	     GtkSourceFold *fold,
	     GtkTextIter   *begin,
	     GtkTextIter   *end)
{
	/* unhide the text after the fold, but still on the same line. */
	if (!gtk_text_iter_starts_line (end))
		gtk_text_iter_forward_line (end);

	_gtk_source_buffer_remove_fold (GTK_SOURCE_BUFFER (buffer),
				      begin, end);

	/* reapply fold to collapsed children. */
	if (fold->children != NULL)
		reapply_fold (buffer, fold->children);

	/* if the fold expansion is animated, the style is gradually
	 * updated from a timeout handler in the view. If it isn't
	 * animated we need to set the style here. This needed when
	 * the user expands the fold using the API instead of the GUI. */
	if (!fold->animated)
		fold->expander_style = GTK_EXPANDER_EXPANDED;
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
	GtkTextBuffer *buffer;
	GtkTextIter begin, end;

	g_return_if_fail (fold != NULL);

	folded = (folded != FALSE);

	if (fold->folded == folded)
		return;

	fold->folded = folded;

	buffer = gtk_text_mark_get_buffer (fold->start_line);

	gtk_text_buffer_get_iter_at_mark (buffer, &begin, fold->start_line);
	gtk_text_buffer_get_iter_at_mark (buffer, &end, fold->end_line);

	if (folded)
		collapse_fold (buffer, fold, &begin, &end);
	else
		expand_fold (buffer, fold, &begin, &end);
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
	GtkTextBuffer *buffer;

	g_return_if_fail (fold != NULL);

	if (gtk_text_mark_get_deleted (fold->start_line))
		g_message ("starting mark DELETED!");

	buffer = gtk_text_mark_get_buffer (fold->start_line);

	if (begin != NULL)
		gtk_text_buffer_get_iter_at_mark (buffer, begin, fold->start_line);
	if (end != NULL)
		gtk_text_buffer_get_iter_at_mark (buffer, end, fold->end_line);
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

	return GTK_SOURCE_BUFFER (gtk_text_mark_get_buffer (fold->start_line));
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
gtk_source_fold_get_lines(GtkSourceFold *fold,
			  GtkTextBuffer *buffer,
			  gint *start_line,
			  gint *end_line) 
{
	GtkTextIter iter_start, iter_stop;
	gtk_text_buffer_get_iter_at_mark (buffer, &iter_start, fold->start_line);
 
	gtk_text_buffer_get_iter_at_mark (buffer, &iter_stop, fold->end_line);
	
	/* The end iter of the fold is on the next line, so if the end
	 * iter is at the start of the line, go back a line. */
	if (gtk_text_iter_starts_line (&iter_stop))
	{
		gtk_text_iter_backward_line (&iter_stop);
	}
	*start_line = gtk_text_iter_get_line(&iter_start);
	*end_line = gtk_text_iter_get_line(&iter_stop);	
}
