/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/* gtksourcefoldmanager.c
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
	/* Create invisibility tag for folding lines. */
//	buffer->priv->fold_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (object),
//				    NULL, "invisible", TRUE, NULL);

#define GTK_SOURCE_FOLD_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManagerPrivate))

struct _GtkSourceFoldManagerPrivate 
{
	GList                 *folds;
	GtkTextTag            *fold_tag;
	gboolean               folds_enabled : 1;
};

G_DEFINE_TYPE (GtkSourceFoldManager, gtk_source_fold_manager, GTK_TYPE_SOURCE_FOLD_MANAGER)

/**
 * insert_child_fold:
 * @buffer: a #GtkSourceBuffer.
 * @child: the new #GtkSourceFold to insert.
 * @parent: the #GtkSourceFold parent to which we are trying to insert the child.
 * @error: a possible #GError returned through the recursive loop.
 *
 * Inserts the specified child #GtkSourceFold into the fold tree somewhere or
 * returns FALSE. In the latter case, a #GError is also set to indicate an
 * illegal operation.
 *
 * This method is called from gtk_source_buffer_add_fold initially with a
 * top-level #GtkSourceFold. After that, the method calls itself recursively
 * until it has either found a #GtkSourceFold where it can insert the child or
 * detected an illegal operation and set @error.
 *
 * Return value: TRUE if the insertion was succesful, FALSE if not. @error is
 * set when FALSE is returned.
 **/
static gboolean
insert_child_fold (GtkSourceBuffer *buffer,
		   GList           *folds,
		   GtkSourceFold   *child,
		   GtkSourceFold   *parent,
		   GError         **error)
{
	GtkTextIter begin, end, pbegin, pend, iter;

	/* If error is set, then return immediately; don't recurse further. */
	if (*error)
		return FALSE;

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
					  &begin, child->start_line);
	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
					  &end, child->end_line);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
					  &pbegin, parent->start_line);
	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
					  &pend, parent->end_line);

	DEBUG (g_message ("insert child (%d, %d) into parent (%d, %d)",
			  gtk_text_iter_get_line (&begin),
			  gtk_text_iter_get_line (&end),
			  gtk_text_iter_get_line (&pbegin),
			  gtk_text_iter_get_line (&pend)));

	/* There are 3 major codepaths in this method:
	 * 1. The child fold falls completely inside the parent fold. Try to add
	 *    the child fold to children of the parent fold recursively. Otherwise
	 *    append the child fold to the parent.
	 * 2. The child fold overlapses the parent fold. This means that the
	 *    start of the child fold is before the start of the parent and the
	 *    end of the child is after the end of the parent. In this case,
	 *    parent & child need to be reparented (parent becomes child and
	 *    child becomes the parent. There's additional logic to check if any
	 *    siblings of the parent need to be reparented.
	 * 3. The child fold intersects with the parent fold. This is an illegal
	 *    operation and will results in FALSE being returned through the
	 *    recursion and the error parameter to be set.
	 */

	/* check if the child fold is within the parent fold. */
	if (gtk_text_iter_compare (&pbegin, &begin) == -1 &&
	    gtk_text_iter_compare (&pend, &end) == 1)
	{
		GList *folds = parent->children;
		GList *last_fold = folds;

		/* fold already has children; try inserting it into one. */
		while (folds != NULL)
		{
			GtkSourceFold *child_fold = folds->data;

			gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
							  &iter, child_fold->start_line);
			/* check if the current fold is past the new fold. */
			if (gtk_text_iter_compare (&iter, &end) == 1)
			{
				DEBUG (g_message ("adding fold before child @ %d",
						  gtk_text_iter_get_line (&iter)));
				parent->children = g_list_insert_before (parent->children,
									 folds, child);
				child->parent = parent;
				return TRUE;
			}

			/* try inserting the child fold recursively. */
			if (insert_child_fold (buffer, child, child_fold, error))
				return TRUE;

			if (*error)
				return FALSE;

			last_fold = folds;
			folds = g_list_next (folds);
		}

		/* Fold is inside parent, but not inside a child of parent.
		 * Append the child fold to the parent. If the child was added
		 * succesfully already, then we never get to this point (we
		 * return in the while loop above). */
		DEBUG (g_message ("adding fold to parent fold @ %d",
				  gtk_text_iter_get_line (&pbegin)));
		if (last_fold == NULL)
			parent->children = g_list_append (NULL, child);
		else
			parent->children = g_list_append (last_fold, child);
		child->parent = parent;
		return TRUE;
	}
	/* check if the child fold overlaps the parent fold. */
	else if (gtk_text_iter_compare (&pbegin, &begin) == 1 &&
		 gtk_text_iter_compare (&pend, &end) == -1)
	{
		GtkSourceFold *sibling;
		GList *siblings, *reparent, *l, *first, *last;

		/* If the parent is a root fold, the "siblings" are actually
		 * the other root folds. Else just get the fold children. */
		if (parent->parent != NULL)
			siblings = g_list_find (parent->parent->children, parent);
		else
			siblings = g_list_find (buffer->priv->folds, parent);

		reparent = g_list_append (NULL, parent);

		DEBUG (g_message ("child overlaps parent; need to reparent..."));

		/* We need to determine which siblings the child overlapses.
		 * Those siblings need to be reparented to the child. If the
		 * child intersects a sibling, then this operation is invalid.
		 *
		 * |		<- parent fold
		 * | |		<- child fold (to be inserted)
		 * | | [	<- first sibling (reparent)
		 * | |
		 * | | [	<- second sibling (reparent)
		 * | |
		 * |
		 * | [		<- third sibling (don't reparent)
		 * |
		 */

		/* The parent fold is the first in the list. Since we've already
		 * added it to the reparent list, we skip it here. */
		siblings = g_list_next (siblings);

		/* First determine which children to reparent. */
		while (siblings != NULL)
		{
			sibling = siblings->data;

			gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
							  &pend, sibling->end_line);

			/* check if we are past the last overlapped sibling. */
			if (gtk_text_iter_compare (&end, &pend) == -1)
			{
				gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
								  &pbegin, sibling->start_line);
				if (gtk_text_iter_compare (&end, &pbegin) != -1)
				{
					g_set_error (error,
						     gtk_source_buffer_error_quark (),
						     0,
						     "Cannot add child fold: new fold [%d-%d] intersects with [%d-%d]",
						     gtk_text_iter_get_line (&begin),
						     gtk_text_iter_get_line (&end),
						     gtk_text_iter_get_line (&pbegin),
						     gtk_text_iter_get_line (&pend));
					g_list_free (reparent);
					return FALSE;
				}

				break;
			}

			DEBUG (g_message ("reparenting @ %d", gtk_text_iter_get_line (&pend)));

			reparent = g_list_prepend (reparent, sibling);
			siblings = g_list_next (siblings);
		}

		/* reparent first sibling to child. */
		reparent = g_list_reverse (reparent);
		sibling = reparent->data;

		/* if sibling->parent is NULL, then it's a root fold. */
		if (sibling->parent == NULL)
		{
			siblings = g_list_find (buffer->priv->folds, sibling);
			g_return_val_if_fail (siblings != NULL, FALSE);
			siblings->data = child;
			child->children = g_list_append (child->children, sibling);
			child->parent = NULL;
			sibling->parent = child;
		}
		else
		{
			siblings = g_list_find (sibling->parent->children, sibling);
			g_return_val_if_fail (siblings != NULL, FALSE);
			siblings->data = child;
			child->children = g_list_append (child->children, sibling);
			child->parent = sibling->parent;
			sibling->parent = child;
		}

		/* reparent all the following siblings as well. */
		l = g_list_next (reparent);
		first = last = NULL;
		while (l != NULL)
		{
			sibling = l->data;

			if (first == NULL)
			{
				if (sibling->parent != NULL)
					first = g_list_find (sibling->parent->children,
							     sibling);
				else
					first = g_list_find (buffer->priv->folds,
							     sibling);

				g_return_val_if_fail (first != NULL, FALSE);

				last = first;
			}

			sibling->parent = child;

			last = g_list_next (last);
			l = g_list_next (l);
		}

		/* Check if there are any siblings left to reparent. */
		if (first != NULL)
		{
			if (first->prev)
				first->prev->next = last;

			if (last)
			{
				last->prev->next = NULL;
				last->prev = first->prev;
			}

			first->prev = NULL;

			child->children = g_list_concat (child->children, first);
		}

		g_list_free (reparent);

		return TRUE;
	}
	else if (gtk_text_iter_in_range (&pbegin, &begin, &end) ||
		 gtk_text_iter_in_range (&pend, &begin, &end) ||
		 gtk_text_iter_equal (&pbegin, &begin) ||
		 gtk_text_iter_equal (&pend, &end))
	{
		g_set_error (error,
			     gtk_source_buffer_error_quark (),
			     0,
			     "Cannot add child fold: new fold [%d-%d] intersects with [%d-%d]",
			     gtk_text_iter_get_line (&begin),
			     gtk_text_iter_get_line (&end),
			     gtk_text_iter_get_line (&pbegin),
			     gtk_text_iter_get_line (&pend));
	}

	return FALSE;
}

GtkSourceFold *
gtk_source_fold_manager_add_fold (GtkSourceFoldManager   *fold_manager,
                                  const GtkTextIter *begin,
                                  const GtkTextIter *end)
{
	DEBUG (g_message ("add fold @ %d, %d",
			  gtk_text_iter_get_line (begin),
			  gtk_text_iter_get_line (end)));

	fold = _gtk_source_fold_new (buffer, begin, end);

	/* Insert the fold either at the root level or as a child of an existing fold. */
	folds = buffer->priv->folds;
	last_fold = folds;
	while (folds != NULL)
	{
		GError *error = NULL;

		parent = folds->data;

		/* check if the current fold is past the new fold. */
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
						  &iter, parent->start_line);
		if (gtk_text_iter_compare (&iter, end) == 1)
		{
			DEBUG (g_message ("adding fold before root %d", gtk_text_iter_get_line (&iter)));
			buffer->priv->folds = g_list_insert_before (buffer->priv->folds,
								    folds, fold);
			break;
		}

		/* try adding the child to all folds in the region. */
		if (insert_child_fold (buffer, fold, parent, &error))
			break;

		if (error != NULL)
		{
	//		g_critical (error->message);
			g_error_free (error);
			gtk_source_fold_free (fold);
			return NULL;
		}

		last_fold = folds;
		folds = g_list_next (folds);
	}

	/* add the fold at the end of the list. */
	if (folds == NULL)
	{
		GList *dummy;

		DEBUG (g_message ("adding fold at end of root"));
		if (last_fold == NULL)
			buffer->priv->folds = g_list_append (NULL, fold);
		else
			dummy = g_list_append (last_fold, fold);
	}

	g_signal_emit (G_OBJECT (buffer), buffer_signals [FOLD_ADDED], 0, fold);

	return fold;
}

void
gtk_source_fold_manager_remove_fold (GtkSourceBuffer *buffer,
                                     GtkSourceFold   *fold)
{
	GList *l;

	g_return_if_fail (GTK_IS_SOURCE_BUFFER (buffer));
	g_return_if_fail (fold != NULL);
	printf(" Removing fold");
	if (fold->folded)
		gtk_source_fold_set_folded (fold, FALSE);

	gtk_text_buffer_delete_mark (GTK_TEXT_BUFFER (buffer), fold->start_line);
	gtk_text_buffer_delete_mark (GTK_TEXT_BUFFER (buffer), fold->end_line);

	l = g_list_find (buffer->priv->folds, fold);
	if (l != NULL)
		buffer->priv->folds = g_list_delete_link (buffer->priv->folds, l);

	g_list_foreach (fold->children, foreach_fold_region, buffer);

	gtk_source_fold_free (fold);
}


/**
 * get_folds_in_region:
 * @buffer: a #GtkSourceBuffer.
 * @begin: the begin point of the region.
 * @end: the end point of the region.
 * @fold: #GtkSourceFold which might or might not lie in the region.
 * @list: the list of #GtkSourceFold's in the region.
 *
 * This method is called from gtk_source_buffer_get_folds_in_region to create a
 * list of #GtkSourceFold's of which the *start* lies in the specified region.
 * This method recurses through the fold hierarchy to create the flattened list.
 * See gtk_source_buffer_get_folds_in_region for more information.
 **/
static void
get_folds_in_region (GtkTextBuffer     *buffer,
		     const GtkTextIter *begin,
		     const GtkTextIter *end,
		     GtkSourceFold     *fold,
		     GList            **list)
{
	GtkTextIter fbegin, fend;
	GList *children;

	gtk_text_buffer_get_iter_at_mark (buffer, &fbegin, fold->start_line);
	gtk_text_buffer_get_iter_at_mark (buffer, &fend, fold->end_line);
	//printf("Found fold between %d and %d\n",
	//	 gtk_text_iter_get_line(&fbegin),
	//	 gtk_text_iter_get_line(&fend));
	/* the region lies in the fold, so add possible children in the region. */
	if (gtk_text_iter_compare (&fbegin, begin) == -1 &&
	    gtk_text_iter_compare (&fend, begin) == 1)
	{
		*list = g_list_append (*list, fold);
		children = fold->children;
		while (!fold->folded && children != NULL)
		{
			get_folds_in_region (buffer, begin, end, children->data, list);
			children = g_list_next (children);
		}
	}
	/* the entire fold lies in the region, so add the fold + children. */
	else if ((gtk_text_iter_compare (&fbegin, begin) >= 0 &&
	          gtk_text_iter_compare (&fend, end) <= 0) ||
	/* start iter is in the region, so add the fold first. */
	         (gtk_text_iter_compare (&fbegin, begin) >= 0 &&
	          gtk_text_iter_compare (&fbegin, end) <= 0))
	{
		*list = g_list_append (*list, fold);
		children = fold->children;
		while (!fold->folded && children != NULL)
		{
			get_folds_in_region (buffer, begin, end, children->data, list);
			children = g_list_next (children);
		}
	}
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
 * Return value: a #GList of the #GtkSourceFold's in the region, or %NULL if
 * there are no folds in the region.
 **/
GList *
_gtk_source_buffer_get_folds_in_region (GtkSourceBuffer   *buffer,
					const GtkTextIter *begin,
					const GtkTextIter *end)
{
	GList *result, *folds;
//	printf("Get folds in region\n");
	g_return_val_if_fail (GTK_IS_SOURCE_BUFFER (buffer), NULL);
	g_return_val_if_fail (begin != NULL && end != NULL, NULL);

	result = NULL;
	folds = buffer->priv->folds;
	while (folds != NULL)
	{
		GtkSourceFold *fold = folds->data;
		GtkTextIter iter;

		/* break when we are past the end of the region. */
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
						  &iter, fold->start_line);
		if (gtk_text_iter_compare (&iter, end) == 1)
			break;

		get_folds_in_region (GTK_TEXT_BUFFER (buffer), begin, end, fold, &result);

		folds = g_list_next (folds);
	}
//	printf("end\n");
	return result;
}

static void
remove_folds_in_region (GtkTextBuffer     *buffer,
			const GtkTextIter *begin,
			const GtkTextIter *end,
			GtkSourceFold     *fold)
{
	GtkTextIter fbegin, fend;
	GList *children;

	gtk_text_buffer_get_iter_at_mark (buffer, &fbegin, fold->start_line);
	gtk_text_buffer_get_iter_at_mark (buffer, &fend, fold->end_line);

	/* the region lies in the fold, so remove possible children in the region. */
	if (gtk_text_iter_compare (&fbegin, begin) == -1 &&
	    gtk_text_iter_compare (&fend, begin) == 1)
	{
		children = fold->children;
		while (!fold->folded && children != NULL)
		{
			remove_folds_in_region (buffer, begin, end, children->data);
			children = g_list_next (children);
		}
	}
	/* start iter is in the region, so add the fold first. */
	else if (gtk_text_iter_compare (&fbegin, begin) >= 0 &&
	         gtk_text_iter_compare (&fbegin, end) <= 0)
	{
		gtk_source_buffer_remove_fold (GTK_SOURCE_BUFFER (buffer), fold);
	}
}

void
gtk_source_buffer_remove_folds_in_region (GtkSourceBuffer   *buffer,
					  const GtkTextIter *begin,
					  const GtkTextIter *end)
{
	GList *folds;

	g_return_if_fail (GTK_IS_SOURCE_BUFFER (buffer));
	g_return_if_fail (begin != NULL && end != NULL);

	folds = buffer->priv->folds;
	while (folds != NULL)
	{
		GtkSourceFold *fold = folds->data;
		GtkTextIter iter;

		/* break when we are past the end of the region. */
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
						  &iter, fold->start_line);
		if (gtk_text_iter_compare (&iter, end) == 1)
			break;

		remove_folds_in_region (GTK_TEXT_BUFFER (buffer), begin, end, fold);

		folds = g_list_next (folds);
	}
}

static GtkSourceFold *
find_fold_at_line (GtkTextBuffer *buffer,
		   GList         *folds,
		   gint           line)
{
	GtkSourceFold *fold;
	GtkTextIter iter;
	gint start_line, end_line;

	while (folds != NULL)
	{
		fold = folds->data;

		gtk_text_buffer_get_iter_at_mark (buffer, &iter, fold->start_line);
		start_line = gtk_text_iter_get_line (&iter);
		gtk_text_buffer_get_iter_at_mark (buffer, &iter, fold->end_line);

		/* The end iter of the fold is on the next line, so if the end
		 * iter is at the start of the line, go back a line. */
		if (gtk_text_iter_starts_line (&iter))
			gtk_text_iter_backward_line (&iter);
		end_line = gtk_text_iter_get_line (&iter);

		if (line >= start_line && line <= end_line)
		{
			GtkSourceFold *child;

			if (!fold->children)
				return fold;

			child = find_fold_at_line (buffer, fold->children, line);
			if (child)
				return child;
			else
				return fold;
		}
		else if (line < start_line)
		{
			return NULL;
		}

		folds = g_list_next (folds);
	}

	return NULL;
}

GtkSourceFold *
_gtk_source_buffer_get_fold_at_line (GtkSourceBuffer *buffer,
				     gint             line)
{
	g_return_val_if_fail (GTK_IS_SOURCE_BUFFER (buffer), NULL);
	g_return_val_if_fail (line >= 0, NULL);

	return find_fold_at_line (GTK_TEXT_BUFFER (buffer),
				  buffer->priv->folds,
				  line);
}

void
_gtk_source_buffer_apply_fold (GtkSourceBuffer	*buffer,
			      const GtkTextIter	*start,
			      const GtkTextIter	*end)
{
	g_return_if_fail (GTK_IS_SOURCE_BUFFER (buffer));

	gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer),
				   buffer->priv->fold_tag,
				   start, end);
}

void
_gtk_source_buffer_remove_fold (GtkSourceBuffer	*buffer,
			      const GtkTextIter	*start,
			      const GtkTextIter	*end)
{
	g_return_if_fail (GTK_IS_SOURCE_BUFFER (buffer));

	gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer),
				    buffer->priv->fold_tag,
				    start, end);
}

static GtkSourceFold *
find_fold_at_iter (GtkTextBuffer     *buffer,
		   GList             *folds,
		   const GtkTextIter *iter)
{
	GtkSourceFold *fold;
	GtkTextIter start_iter, end_iter;

	while (folds != NULL)
	{
		fold = folds->data;

		gtk_text_buffer_get_iter_at_mark (buffer, &start_iter, fold->start_line);
		gtk_text_buffer_get_iter_at_mark (buffer, &end_iter, fold->end_line);

		if (gtk_text_iter_compare (&start_iter, iter) <= 0 &&
		    gtk_text_iter_compare (&end_iter, iter) >= 0)
		{
			GtkSourceFold *child;

			if (!fold->children)
				return fold;

			child = find_fold_at_iter (buffer, fold->children, iter);
			if (child)
				return child;
			else
				return fold;
		}
		else if (gtk_text_iter_compare (iter, &start_iter) == -1)
		{
			return NULL;
		}

		folds = g_list_next (folds);
	}

	return NULL;
}

GtkSourceFold *
gtk_source_buffer_get_fold_at_iter (GtkSourceBuffer   *buffer,
				    const GtkTextIter *iter)
{
	g_return_val_if_fail (GTK_IS_SOURCE_BUFFER (buffer), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	return find_fold_at_iter (GTK_TEXT_BUFFER (buffer),
				  buffer->priv->folds,
				  iter);
}

const GList *
gtk_source_buffer_get_root_folds (GtkSourceBuffer *buffer)
{
	g_return_val_if_fail (GTK_IS_SOURCE_BUFFER (buffer), NULL);

	return buffer->priv->folds;
}

/////////////////////////
Destroy!
static void
foreach_fold_region (gpointer data, gpointer user_data)
{
	gtk_source_buffer_real_remove_fold (GTK_SOURCE_BUFFER (user_data), data);
}

	/* Remove all existing folds if folds are disabled.
	 * We should not remove the folds! */
	if (!folds_enabled && buffer->priv->folds != NULL)
	{
		GList *folds = g_list_copy (buffer->priv->folds);
		g_list_foreach (folds, foreach_fold_region, buffer);
		g_list_free (folds);
	}
/////////////////////////
