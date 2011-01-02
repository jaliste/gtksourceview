/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/* gtksourcefoldmanager.c
 * This file is part of GtkSourceView
 *
 * Copyright (C) 1999-2002 - Jeroen Zwartepoorte <jeroen@xs4all.nl>
 * Copyright (C) 2010-2011 - José Aliste <jaliste@src.gnome.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include "gtksourcefold-private.h"
#include "gtksourcefoldmanager.h"

/*
#define ENABLE_DEBUG
#define ENABLE_PROFILE
*/
#undef ENABLE_DEBUG
#undef ENABLE_PROFILE

#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

#ifdef ENABLE_PROFILE
#define PROFILE(x) (x)
#else
#define PROFILE(x)
#endif

#define GTK_SOURCE_FOLD_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManagerPrivate))

struct _GtkSourceFoldManagerPrivate 
{
	GtkSourceBuffer       *buffer;
	GSequence             *folds;
	GtkTextTag            *fold_tag;
};

G_DEFINE_TYPE (GtkSourceFoldManager, gtk_source_fold_manager, G_TYPE_OBJECT)

static gint
compare_folds (gpointer lhs, gpointer rhs, gpointer data)
{
	GtkTextIter iter_lhs, iter_rhs;

	gtk_source_fold_get_bounds ((GtkSourceFold *)lhs, &iter_lhs, NULL);
	gtk_source_fold_get_bounds ((GtkSourceFold *)rhs, &iter_rhs, NULL);

	return gtk_text_iter_compare (&iter_lhs, &iter_rhs);
}

static gint 
compare_folds_inv (gpointer lhs, gpointer rhs, gpointer data)
{
	return (-1)*compare_folds (lhs, rhs, data);
}
GtkSourceFold *
gtk_source_fold_manager_add_fold (GtkSourceFoldManager *manager,
                                  const GtkTextIter    *begin,
                                  const GtkTextIter    *end)
{
	GtkSourceFold *new_fold;
	GSequenceIter *insert_iter, *previous_iter;
	GtkSourceFold *closest_fold;
	GtkTextIter closest_begin, closest_end;

	new_fold = _gtk_source_fold_new (manager->priv->buffer, begin, end);

	insert_iter = g_sequence_search (manager->priv->folds,
                                         new_fold, (GCompareDataFunc) compare_folds,
                                         NULL);

	/* insert_iter is a GSequenceIter pointing to the first fold having a 
	 * start mark  larger than the start_mark of the fold to be inserted. 
	 * If such fold does not exist, then insert_iter is the end iter of the 
	 * sequence.
	 */

	if (!g_sequence_iter_is_begin (insert_iter))
	{
		GtkSourceFold *previous;
		GtkTextIter prev_begin, prev_end;

		/* We look for the parent of the fold to be inserted. 
		   Since the folds are ordered by start_marks and can only 
		   overlap if one is contained in another, the parent is either 
		   the previous fold, or the parent of the previous fold */

		previous_iter = g_sequence_iter_prev (insert_iter);
		previous = g_sequence_get (previous_iter);
		gtk_source_fold_get_bounds (previous, &prev_begin, &prev_end);

		/* ASSERT previous->begin <= begin  */

		if (gtk_text_iter_compare (&prev_end, begin) > 0)
		{
			if (gtk_text_iter_compare (&prev_end, end) > 0 )
			{
				/* Previous is parent */
				new_fold->parent = previous;
			}
			else
			{
				/* overlap is not trivial, fold is invalid */
				/* FIXME: I leak the new_fold here */
				return NULL;
			}
		}
		else if (previous->parent != NULL &&
		         gtk_source_fold_compare_end_with_iter (previous->parent, end) >= 0)
		{
			/* previous and new_fold don't overlap, only 
			 * possible parent is previous's parent */

			new_fold->parent = previous->parent;
		} 
	}

	if (!g_sequence_iter_is_end (insert_iter))
	{
		closest_fold = g_sequence_get (insert_iter);
		gtk_source_fold_get_bounds (closest_fold, &closest_begin, &closest_end);

		/* ASSERT: insert->iter->begin > begin */
		if (gtk_text_iter_compare (&closest_begin, end) >= 0)
		{	
			/* no overlap so new_fold has no childs */
			new_fold->node = g_sequence_insert_before (insert_iter, new_fold);
		} 
		else if (gtk_text_iter_compare (&closest_end, end) <= 0)
		{
			/* insert_iter is contained in new_fold
			   we need to reparent. */
			/* FIXME: Add reparenting method */
		}
		else 
		{
			/* overlap is not trivial, fold is invalid */
			/* FIXME: I Am leaking here */
			return NULL;
		}
	}
	else
	{
		new_fold->node = g_sequence_append (manager->priv->folds, new_fold);
	} 

}

void 
foreach_children_reparent (GtkSourceFold *fold, GtkSourceFold *child)
{
	child->parent = fold->parent;
}

void
gtk_source_fold_manager_remove_fold (GtkSourceFoldManager *manager,
                                     GtkSourceFold        *fold)
{
	g_return_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager));
	g_return_if_fail (fold != NULL);

	if (fold->folded)
		gtk_source_fold_manager_expand_fold (manager, fold);

	if (!g_sequence_iter_is_end (fold->node))
	{
		GSequenceIter *last_child;

		last_child = g_sequence_iter_prev (gtk_source_fold_next (fold, TRUE));
		g_sequence_foreach_range (fold->node, last_child, (GFunc) foreach_children_reparent, fold);
	}
	g_sequence_remove (fold->node);
	/* FIXME */
	//gtk_source_fold_free (fold);
}

/**
 * gtk_source_buffer_get_folds_in_region:
 * @buffer: a #GtkSourceBuffer.
 * @begin: the begin point of the region.
 * @end: the end point of the region.
 *
 * Returns a list of all folds in the specified region. This is a flattened list
 * of the parent->child fold hierarchy. This function is mainly used in
 * gtk_source_view_get_lines to determine which folds to draw.
 *
 * This method returns the folds of which the start lies in the region. So if
 * a fold begins before the region, the fold itself isn't returned, but its
 * children might.
 *
 * Return value: a Flattened?? #GList of the #GtkSourceFold's in the region, or %NULL if
 * there are no folds in the region.
 **/

GList *
gtk_source_fold_manager_get_folds_in_region (GtkSourceFoldManager *manager,
					     const GtkTextIter    *begin,
					     const GtkTextIter    *end)
{
	GList *result = NULL;
	GSequence *list_folds = manager->priv->folds;
	GtkSourceBuffer *buffer = manager->priv->buffer;
	GSequenceIter *iter, *iter2;
	GtkSourceFold *fold, *tmp_fold;
	GtkTextIter iter_start, iter_end;


	/* Create a tmp_fold so we can search for the first
	 * fold whose start_mark appears after @begin. */
	tmp_fold = _gtk_source_fold_new (GTK_SOURCE_BUFFER (buffer), begin, end);
	iter = g_sequence_search (list_folds, tmp_fold, (GCompareDataFunc) compare_folds, GINT_TO_POINTER (-1));
	iter2 = iter;
	/* FIXME */
	/* Do not leak temp_fold */
	/* gtk_source_fold_free (tmp_fold); */

	/* First we go forwards from iter in the sequence of folds until we find 
	 * a fold whose start_mark appears after @end.
	 * The order implies that all the folds we traversed intersect the region, 
	 * but the last one. */
	while ( !g_sequence_iter_is_end (iter)) 
	{
		fold = g_sequence_get (iter);
		gtk_source_fold_get_bounds (fold, &iter_start, &iter_end);

		if (gtk_text_iter_compare (&iter_start, end) > 0)
		{
			break;
		}
		result = g_list_prepend (result, fold);
		iter = g_sequence_iter_next (iter);
	}
	result = g_list_reverse (result);

	/* Now we go backwards from iter2 until we find a fold whose end mark appears 
	 * before @begin. The order implies that all the folds traversed
	 * intersect the region */
	while (!g_sequence_iter_is_begin (iter2))
	{
		iter2 = g_sequence_iter_prev (iter2);
		fold = g_sequence_get (iter2);
		gtk_source_fold_get_bounds (fold, &iter_start, &iter_end);
		if (gtk_text_iter_compare (&iter_end, begin) < 0)
		{
			/* This fold does not intersect the region but maybe its 
			 * parent does */
			while (fold->parent != NULL)
			{
				if (gtk_source_fold_compare_end_with_iter (fold->parent, begin)>=0)
				{
					gint a,b;
					gtk_source_fold_get_lines (fold->parent,&a, &b);
					/* parent stops inside the region, add it to the list.*/
					result = g_list_prepend (result, fold->parent);
				}
				fold = fold->parent;
			}
			break;
		}

		result = g_list_prepend (result, fold);
	}
	return result;
}

void
gtk_source_fold_manager_remove_folds_in_region (GtkSourceFoldManager *manager,
					        const GtkTextIter    *begin,
					        const GtkTextIter    *end)
{
	g_return_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager));
	g_return_if_fail (begin != NULL && end != NULL);

	/* FIXME : IMPLEMENT */
}

static void
collapse_children (GtkSourceFoldManager *manager,
                   GtkSourceFold *fold)
{
	/* Compute the children */
	GtkTextIter begin, end;
	GSequenceIter *node;

	node  = g_sequence_iter_next (fold->node);
	gtk_source_fold_get_bounds (fold, &begin, &end);

	while (!g_sequence_iter_is_end (node))
	{
		GtkSourceFold *child = g_sequence_get (node);
		GtkTextIter cbegin, cend;

		gtk_source_fold_get_bounds (child, &cbegin, &cend);
		if (gtk_text_iter_compare (&end, &cbegin) < 0)
		{
			break;
		}

		if (gtk_source_fold_get_folded (child))
		{
			gtk_source_fold_manager_collapse_fold (manager, child);
		}
		node  = g_sequence_iter_next (node);
	}
}

void
gtk_source_fold_manager_expand_fold (GtkSourceFoldManager *manager, 
                                     GtkSourceFold *fold)
{
	GtkTextIter start, end;

	gtk_source_fold_get_bounds (fold, &start, &end);

	/* unhide the text after the fold, but still on the same line. */
	if (!gtk_text_iter_starts_line (&end))
		gtk_text_iter_forward_line (&end);

	gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (manager->priv->buffer),
	                            manager->priv->fold_tag,
	                            &start, &end);
	gtk_source_fold_set_folded (fold, FALSE);

	/* Hide collapsed children. */
	collapse_children (manager, fold);
}

void
gtk_source_fold_manager_collapse_fold (GtkSourceFoldManager *manager,
                                       GtkSourceFold *fold)
{
	GtkTextIter start, end;
	GtkTextIter insert;
	GtkTextMark *insert_mark;
	GtkTextBuffer *text_buffer;

	text_buffer = GTK_TEXT_BUFFER (manager->priv->buffer);

	gtk_source_fold_get_bounds (fold, &start, &end);

	/* if the starting point of the fold has no text before it on the line,
	 * then only hide part of the line so the user still sees something. */
	if (gtk_text_iter_starts_line (&start))
		gtk_text_iter_forward_to_line_end (&start);

	/* hide the entire line that contains the end of the fold. */
	if (!gtk_text_iter_starts_line (&end))
		gtk_text_iter_forward_line (&end);

	gtk_text_buffer_apply_tag (text_buffer, manager->priv->fold_tag, &start, &end);
	gtk_source_fold_set_folded (fold, TRUE);

	insert_mark = gtk_text_buffer_get_insert (text_buffer);
	gtk_text_buffer_get_iter_at_mark (text_buffer, &insert, insert_mark);

	/* make the cursor visible again if it was inside the fold. */
	if (gtk_text_iter_in_range (&insert, &start, &end))
	{
		if (!gtk_text_iter_forward_visible_cursor_position (&insert))
			gtk_text_iter_backward_visible_cursor_position (&insert);

		gtk_text_buffer_place_cursor (text_buffer, &insert);
	}
}

void
gtk_source_fold_manager_set_fold_tag  (GtkSourceFoldManager *manager, GtkTextTag *tag)
{
	manager->priv->fold_tag = tag;
}

static void
gtk_source_fold_manager_class_init (GtkSourceFoldManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

//	object_class->finalize = gtk_source_gutter_renderer_folds_finalize;

	g_type_class_add_private (object_class, sizeof (GtkSourceFoldManagerPrivate));
}

static void
gtk_source_fold_manager_init (GtkSourceFoldManager *manager)
{
	manager->priv = G_TYPE_INSTANCE_GET_PRIVATE (manager, GTK_TYPE_SOURCE_FOLD_MANAGER,
	                                             GtkSourceFoldManagerPrivate);
	manager->priv->folds = g_sequence_new (NULL);
}

GtkSourceFoldManager *
gtk_source_fold_manager_new (GtkSourceBuffer *buffer)
{
	GtkSourceFoldManager *manager = g_object_new (GTK_TYPE_SOURCE_FOLD_MANAGER, NULL);

	manager->priv->buffer = g_object_ref (buffer);

	return manager;
}
