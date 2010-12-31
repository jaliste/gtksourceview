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

#define GTK_SOURCE_FOLD_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_SOURCE_FOLD_MANAGER, GtkSourceFoldManagerPrivate))

struct _GtkSourceFoldManagerPrivate 
{
	GtkSourceBuffer       *buffer;
	GList                 *folds;
	GtkTextTag            *fold_tag;
	gboolean               folds_enabled : 1;
};

enum 
{
	SUPERSET,
	SUBSET,
	INVALID,
	EMPTY_SET
};

G_DEFINE_TYPE (GtkSourceFoldManager, gtk_source_fold_manager, G_TYPE_OBJECT)

static GQuark
gtk_source_buffer_error_quark (void)
{
	static GQuark q = 0;

	if (q == 0)
		q = g_quark_from_static_string ("gtk-source-buffer-error-quark");

	return q;
}
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

static GtkSourceFold *
reparent (GtkSourceFold *fold,
          GtkSourceFold *new_parent)
{
	GtkSourceFold *old_parent;

	old_parent = fold->parent;
	fold->parent = new_parent;
	new_parent->last_child = g_list_append (new_parent->last_child, fold);
	if (new_parent->children == NULL)
	{
		new_parent->children = new_parent->last_child;
	}
	return old_parent;
}
 
static GList *
insert_fold (GtkSourceFold *child,
	     GtkTextIter   *c_begin,
	     GtkTextIter   *c_end,
	     GList         *root_folds,
	     GError         **error)
{
	GList *folds, *saved_fold, last_fold;
	GtkSourceFold *cur_fold;
	gboolean has_reparented = FALSE;
	GtkTextIter child_begin, child_end;

	child_begin = *c_begin;
	child_end = *c_end;

	folds = root_folds;
	GtkTextIter begin, end, iter;

	/* If error is set, then return immediately; don't recurse further. */
//	if (*error)
//		return root_folds;
	if (root_folds == NULL)
	{
		printf("0\n");
		root_folds = g_list_append (root_folds, child);
		return root_folds;
	}
	while (folds != NULL) 
	{
		cur_fold = folds->data;
		gtk_source_fold_get_bounds (cur_fold, &begin, &end);
		gtk_source_fold_get_bounds (child, &child_begin, &child_end);

		printf ("CUR_FOLD [%d,%d]\n", gtk_text_iter_get_line (&begin), gtk_text_iter_get_line (&end));
		printf ("CHILD FOLD [%d, %d]\n", gtk_text_iter_get_line (&child_begin), gtk_text_iter_get_line 
		if (gtk_text_iter_compare (&child_begin, &begin) >=0 &&
		    gtk_text_iter_compare (&child_end, &end) <= 0)
		{
			printf("A\n");
			/* child is inside cur_fold */
			cur_fold->children = insert_fold (child, &child_begin, &child_end, cur_fold->children, error);
 
			return root_folds;
		} 
		else if (gtk_text_iter_compare (&child_begin, &begin) <=0 &&
			 gtk_text_iter_compare (&child_end, &end) >=0) 
		{
			printf("B\n");
			/* cur fold is inside child. */
			if (!has_reparented) 
			{
				saved_fold = g_list_previous (folds);
				has_reparented = TRUE;
			}
			root_folds = g_list_delete_link (root_folds, folds);
			reparent (cur_fold, child); 
			folds = saved_fold;
		}
		else if (gtk_text_iter_compare (&child_begin, &end) <= 0)
		{
			printf("C\n");
			if (has_reparented)
			{
				printf("D\n");
				root_folds = g_list_insert_before (root_folds, g_list_next (saved_fold), child);
				return root_folds;
				/* MUST DELETE ALL THE LINKS */
			}
			/* cur fold starts after child ends */
		} else {
			printf("D\n");
		}
		folds = g_list_next (folds);
	}
		if (has_reparented)
			{
				root_folds = g_list_insert_before (root_folds, g_list_next (saved_fold), child);
				return root_folds;
				/* MUST DELETE ALL THE LINKS */
			}
		

	return root_folds;
}
#if 0			
sibling = siblings->data;

			gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
							  &pend, sibling->end_mark);

			/* check if we are past the last overlapped sibling. */
			if (gtk_text_iter_compare (&end, &pend) == -1)
			{
				gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
								  &pbegin, sibling->start_mark);
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


		if (gtk_text_iter_compare (&child_begin, &end) > 0)
		{ 

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
			if (insert_child_fold (buffer, child, child_fold, root_folds, error))
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
			siblings = g_list_find (root_folds, parent);

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
		}

		/* reparent first sibling to child. */
		reparent = g_list_reverse (reparent);
		sibling = reparent->data;

		/* if sibling->parent is NULL, then it's a root fold. */
		if (sibling->parent == NULL)
		{
			siblings = g_list_find (root_folds, sibling);
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
					first = g_list_find (root_folds,
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
#endif 

static traverse_folds (GList *folds, gchar *tabs)
{
	GList *iter_folds = folds;
	while (iter_folds != NULL)
	{
		gint start, end;
		GtkSourceFold *fold = iter_folds->data;


		gtk_source_fold_get_lines (fold, &start, &end);
		printf ("%s FOLD [%d,%d]\n",tabs, start, end);
		if (fold->children) {
			tabs = g_strdup_printf ("\t%s", tabs);
			traverse_folds (fold->children, tabs);
			g_free (tabs);
		}
		iter_folds = g_list_next (iter_folds);
	}
}
GtkSourceFold *
gtk_source_fold_manager_add_fold (GtkSourceFoldManager *manager,
                                  const GtkTextIter    *begin,
                                  const GtkTextIter    *end)
{
	GtkSourceFold *new_fold;
	GtkSourceBuffer *buffer;
	GList *last_fold, *fold_list;
	GtkTextIter iter;
	GtkTextIter new_begin, new_end;

	g_message ("add fold @ %d, %d",
			  gtk_text_iter_get_line (begin),
			  gtk_text_iter_get_line (end));

	buffer = manager->priv->buffer;
	new_fold = _gtk_source_fold_new (buffer, begin, end);

	/* Insert the fold either at the root level or as a child of an existing fold. */
	fold_list = manager->priv->folds;
	last_fold = fold_list;
	manager->priv->folds = insert_fold (new_fold, begin, end, manager->priv->folds, NULL);
	traverse_folds (manager->priv->folds, g_strdup (""));	
		
#if 0
	while (fold_list != NULL)
	{
		GtkSourceFold *parent;
		GError *error = NULL;

		parent = fold_list->data;

		gtk_source_fold_get_bounds (parent, &iter, NULL);
		if (gtk_text_iter_compare (&iter, end) == 1)
		{
			/* current fold starts after the end of the new fold */
			DEBUG (g_message ("adding fold before root %d", gtk_text_iter_get_line (&iter)));
			manager->priv->folds = g_list_insert_before (manager->priv->folds,
								     fold_list, new_fold);
			break;
		}

		/* try adding the child to all folds in the region. */
		if (insert_child_fold (buffer, new_fold, parent, manager->priv->folds, &error))
			break;

		if (error != NULL)
		{
	//		g_critical (error->message);
			g_error_free (error);
			gtk_source_fold_free (new_fold);
			return NULL;
		}

		last_fold = fold_list;
		fold_list = g_list_next (fold_list);
	}

	/* add the fold at the end of the list. */
	if (fold_list == NULL)
	{
		GList *dummy;

		DEBUG (g_message ("adding fold at end of root"));
		if (last_fold == NULL)
			manager->priv->folds = g_list_append (NULL, new_fold);
		else
			dummy = g_list_append (last_fold, new_fold);
	}
#endif
	g_signal_emit_by_name (G_OBJECT (buffer), "fold-added", 0, new_fold);

	return new_fold;
}

void
gtk_source_fold_manager_remove_fold (GtkSourceFoldManager *manager,
                                     GtkSourceFold        *fold)
{
	GList *l;
	GtkTextBuffer *buffer;

	g_return_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager));
	g_return_if_fail (fold != NULL);

	buffer = GTK_TEXT_BUFFER (manager->priv->buffer);
	if (fold->folded)
		gtk_source_fold_set_folded (fold, FALSE);

	gtk_text_buffer_delete_mark (buffer, fold->start_mark);
	gtk_text_buffer_delete_mark (buffer, fold->end_mark);

	l = g_list_find (manager->priv->folds, fold);
	if (l != NULL)
		manager->priv->folds = g_list_delete_link (manager->priv->folds, l);
// REMOVE CHILDREN FOLDS???
//	g_list_foreach (fold->children, foreach_fold_region, manager);

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

	gtk_text_buffer_get_iter_at_mark (buffer, &fbegin, fold->start_mark);
	gtk_text_buffer_get_iter_at_mark (buffer, &fend, fold->end_mark);
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
 * Return value: a Flattened?? #GList of the #GtkSourceFold's in the region, or %NULL if
 * there are no folds in the region.
 **/
GList *
gtk_source_fold_manager_get_folds_in_region (GtkSourceFoldManager *manager,
					      const GtkTextIter    *begin,
					      const GtkTextIter    *end)
{
	GList *result, *folds;
	GtkSourceBuffer *buffer;

	g_return_val_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager), NULL);
	g_return_val_if_fail (begin != NULL && end != NULL, NULL);

	buffer = manager->priv->buffer;
	result = NULL;
	folds = manager->priv->folds;
	while (folds != NULL)
	{
		GtkSourceFold *fold = folds->data;
		GtkTextIter iter;

		/* break when we are past the end of the region. */
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
						  &iter, fold->start_mark);
		if (gtk_text_iter_compare (&iter, end) == 1)
			break;

		get_folds_in_region (GTK_TEXT_BUFFER (buffer), begin, end, fold, &result);

		folds = g_list_next (folds);
	}

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

	gtk_text_buffer_get_iter_at_mark (buffer, &fbegin, fold->start_mark);
	gtk_text_buffer_get_iter_at_mark (buffer, &fend, fold->end_mark);

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
		//remove_fold (GTK_SOURCE_BUFFER (buffer), fold);
	}
}

void
gtk_source_fold_manager_remove_folds_in_region (GtkSourceFoldManager *manager,
					        const GtkTextIter    *begin,
					        const GtkTextIter    *end)
{
	GList *folds;
	GtkSourceBuffer *buffer;

	g_return_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager));
	g_return_if_fail (begin != NULL && end != NULL);

	folds = manager->priv->folds;
	buffer = manager->priv->buffer;

	while (folds != NULL)
	{
		GtkSourceFold *fold = folds->data;
		GtkTextIter iter;

		/* break when we are past the end of the region. */
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
						  &iter, fold->start_mark);
		if (gtk_text_iter_compare (&iter, end) == 1)
			break;

		remove_folds_in_region (GTK_TEXT_BUFFER (buffer), begin, end, fold);

		folds = g_list_next (folds);
	}
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

		gtk_source_fold_get_bounds (fold, &start_iter, &end_iter);

		if (gtk_text_iter_compare (&start_iter, iter) <= 0 &&
		    gtk_text_iter_compare (&end_iter, iter) >= 0)
		{
			GtkSourceFold *child;

			if (!fold->children)
				return fold;

			child = find_fold_at_iter (buffer, fold->children, iter);

			return (child) ? child : fold;
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
gtk_source_fold_manager_get_fold_at_line (GtkSourceFoldManager *manager,
					  gint                  line)
{
	GtkTextIter iter;
	g_return_val_if_fail (GTK_IS_SOURCE_FOLD_MANAGER (manager), NULL);
	g_return_val_if_fail (line >= 0, NULL);

	gtk_text_iter_set_line (&iter, line);

	return find_fold_at_iter (GTK_TEXT_BUFFER (manager->priv->buffer),
	                          manager->priv->folds, &iter);
}

/*
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
*//*
const GList *
gtk_source_buffer_get_root_folds (GtkSourceBuffer *buffer)
{
	g_return_val_if_fail (GTK_IS_SOURCE_BUFFER (buffer), NULL);

	return buffer->priv->folds;
}
*/
/////////////////////////
static void
foreach_fold_region (gpointer data, gpointer user_data)
{
	gtk_source_buffer_real_remove_fold (GTK_SOURCE_BUFFER (user_data), data);
}

	/* Remove all existing folds if folds are disabled.
	 * We should not remove the folds! */
/*	if (!folds_enabled && buffer->priv->folds != NULL)
	{
		GList *folds = g_list_copy (buffer->priv->folds);
		g_list_foreach (folds, foreach_fold_region, buffer);
		g_list_free (folds);
	}*/
/////////////////////////

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
}

GtkSourceFoldManager *
gtk_source_fold_manager_new (GtkSourceBuffer *buffer)
{
	GtkSourceFoldManager *manager = g_object_new (GTK_TYPE_SOURCE_FOLD_MANAGER, NULL);
	manager->priv->buffer = buffer;

	return manager;
}
