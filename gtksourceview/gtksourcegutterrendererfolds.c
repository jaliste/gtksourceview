/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/* gtksourcegutterrendererfolds.c
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - José Aliste <jaliste@src.gnome.org>
 *                      Garrett Regier <alias301@gmail.com>
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

#include "gtksourcegutterrendererfolds.h"
#include "gtksourceview-typebuiltins.h"
#include "gtksourcefold.h"
#include "gtksourcebuffer-private.h"

#define MAX_DEPTH	20
#define FIXED_WIDTH	10
#define FIXED_HEIGHT    12

#define GTK_SOURCE_GUTTER_RENDERER_FOLDS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, GtkSourceGutterRendererFoldsPrivate))

struct _GtkSourceGutterRendererFoldsPrivate
{
	GtkSourceFoldMarkType fold_mark;
	guint                 depth;
	GList                *folds_visible;
	gint                  line_height;
	gint                  length_expander;
};

G_DEFINE_TYPE (GtkSourceGutterRendererFolds, gtk_source_gutter_renderer_folds, GTK_TYPE_SOURCE_GUTTER_RENDERER)

static int
get_line_height (GtkSourceView *view)
{
	PangoLayout *layout;
	gint height = FIXED_HEIGHT;

	layout = gtk_widget_create_pango_layout (GTK_WIDGET (view), "QWERTY");

	if (layout)
	{
		pango_layout_get_pixel_size (layout, NULL, &height);
		g_object_unref (layout);
	}

	return height;
}

static void
gutter_renderer_folds_begin (GtkSourceGutterRenderer      *renderer,
                             cairo_t                      *cr,
                             GdkRectangle                 *background_area,
                             GdkRectangle                 *cell_area,
                             GtkTextIter                  *start,
                             GtkTextIter                  *end)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkTextView *view;
	GtkTextBuffer *buffer;
	GList *folds_in_region = NULL;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);
	view = gtk_source_gutter_renderer_get_view (renderer);
	buffer = gtk_text_view_get_buffer (view);

	folds_in_region = gtk_source_buffer_get_folds_in_region (GTK_SOURCE_BUFFER (buffer), start, end);

	folds_renderer->priv->folds_visible = folds_in_region;
	folds_renderer->priv->line_height = get_line_height (GTK_SOURCE_VIEW (view));

	/* TODO: Make the length of the expander dependent  on the size of the font? */
	folds_renderer->priv->length_expander = FIXED_WIDTH / 2;
}

static void
gutter_renderer_folds_draw (GtkSourceGutterRenderer *renderer,
                            cairo_t                 *cr,
                            GdkRectangle            *background_area,
                            GdkRectangle            *cell_area,
                            GtkTextIter             *start,
                            GtkTextIter             *end,
                            GtkSourceGutterRendererState state)
{
	GtkTextView *view;
	GtkTextBuffer *buffer;
	GtkSourceGutterRendererFolds *renderer_fold;
	gint c_x, c_y, l, xpad;

	/* Chain up to draw background */
	GTK_SOURCE_GUTTER_RENDERER_CLASS (
		gtk_source_gutter_renderer_folds_parent_class)->draw (renderer,
		                                                     cr,
		                                                     background_area,
		                                                     cell_area,
		                                                     start,
		                                                     end,
		                                                     state);

	renderer_fold = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);

	if (renderer_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_NONE)
		return;

	view = gtk_source_gutter_renderer_get_view (renderer);
	buffer = gtk_text_view_get_buffer (view);
	
	/* FIXME: set the correct color  using Style Context
	context = gtk_widget_get_style_context (GTK_WIDGET (view));
	gdk_cairo_set_source_color (cr, &style->fg[state]);
	*/

	cairo_set_line_width (cr, 1);

	g_object_get (G_OBJECT (renderer), "xpad", &xpad, NULL);

	c_x = cell_area->x + cell_area->width / 2;
	c_y = cell_area->y + renderer_fold->priv->line_height / 2;
	l = renderer_fold->priv->length_expander;

	switch (renderer_fold->priv->fold_mark)
	{
		case GTK_SOURCE_FOLD_MARK_INTERIOR:
			cairo_move_to (cr, c_x + 0.5, cell_area->y);
			cairo_rel_line_to (cr, 0, cell_area->height);
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_STOP:
			cairo_move_to (cr, c_x + 0.5, cell_area->y);
			cairo_rel_line_to (cr, 0, cell_area->height - renderer_fold->priv->line_height / 2.0);
			cairo_rel_line_to (cr, cell_area->width / 2.0, 0);

			if (renderer_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + 0.5, c_y);
				cairo_line_to (cr, c_x + 0.5, cell_area->y + cell_area->height);
			}
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_START_FOLDED:
			cairo_move_to (cr, c_x + 0.5 , c_y - l + xpad + 0.5);
			cairo_rel_line_to (cr, 0, 2 * (l - xpad));
		case GTK_SOURCE_FOLD_MARK_START:
			if (renderer_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + 0.5, cell_area->y);
				cairo_line_to (cr, c_x + 0.5, c_y + 0.5 - l);
			}
			cairo_move_to (cr, c_x + 0.5, c_y + l + 0.5);
			cairo_line_to (cr, c_x + 0.5, cell_area->y + cell_area->height);

			if (state & GTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT)
			{
				cairo_save (cr);
				/* FIXME: Set the correct color using style context *
				gdk_cairo_set_source_color (cr, &style->light[state]);
				*/
				cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);
				cairo_fill (cr);
				cairo_restore (cr);
			}
			cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);
			cairo_move_to (cr, c_x + xpad + 0.5 - l, c_y + 0.5);
			cairo_rel_line_to (cr, 2 * (l - xpad), 0);
			cairo_stroke (cr);
			break;

		default:
			break;
	}
}

static void
gutter_renderer_folds_activate (GtkSourceGutterRenderer *renderer,
                                GtkTextIter             *iter,
                                GdkRectangle            *area,
                                GdkEvent                *event)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkSourceFold *fold;
	GList *folds;
	gint line_number;
	GtkSourceBuffer *buffer;
	GtkTextView *view;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);
	view = GTK_TEXT_VIEW (gtk_source_gutter_renderer_get_view (renderer));
	buffer = gtk_text_view_get_buffer (view);
	line_number = gtk_text_iter_get_line (iter);
	folds = folds_renderer->priv->folds_visible;

	while (folds != NULL)
	{
		gint start_line, end_line;

		fold = folds->data;
		gtk_source_fold_get_lines (fold, &start_line, &end_line);

		if (line_number == start_line && fold != NULL)
		{
			if (gtk_source_fold_get_folded (fold)) 
			{
				gtk_source_buffer_fold_expand (buffer, fold);
			} else {
				gtk_source_buffer_fold_collapse (buffer, fold);
			}
			break;
		}
		folds = g_list_next (folds);
	}
}

static gboolean
gutter_renderer_folds_query_activatable (GtkSourceGutterRenderer *renderer,
                                         GtkTextIter             *iter,
                                         GdkRectangle            *area,
                                         GdkEvent                *event)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkSourceFold *fold;
	GList *folds;
	gint line_number;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);

	folds = folds_renderer->priv->folds_visible;
	line_number = gtk_text_iter_get_line (iter);

	while (folds != NULL)
	{
		gint start_line, end_line;

		fold = folds->data;
		gtk_source_fold_get_lines (fold, &start_line, &end_line);

		if (line_number == start_line && fold != NULL)
		{
			double x, y, x_c, y_c;
			int l = folds_renderer->priv->length_expander;

			/* Ensure that the pointer is over the expander */
			gdk_event_get_coords (event, &x, &y);
			x_c = area->x + area->width / 2;
			y_c = area->y + folds_renderer->priv->line_height / 2;

			return  (MAX (ABS (x_c - x), ABS (y_c - y)) <= l);
		}
		folds = g_list_next (folds);
	}

	return FALSE;
}

static void
calculate_size (GtkSourceGutterRenderer *renderer)
{
	guint xpad;

	g_object_get (G_OBJECT (renderer), "xpad", &xpad, NULL);
	gtk_source_gutter_renderer_set_size (renderer, (gint) xpad * 2 + FIXED_WIDTH);

}

static void
gutter_renderer_folds_query_data (GtkSourceGutterRenderer      *renderer,
                                  GtkTextIter                  *start,
                                  GtkTextIter                  *end,
                                  GtkSourceGutterRendererState  state)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GList *folds;
	gint line_number;
	GtkSourceFoldMarkType fold_mark;
	guint depth;

	line_number = gtk_text_iter_get_line (start);

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);
	folds = folds_renderer->priv->folds_visible;

	fold_mark = GTK_SOURCE_FOLD_MARK_NONE;
	depth = 0;

	while (folds != NULL)
	{
		gint start_line, end_line;
		GtkSourceFold *fold = folds->data;

		gtk_source_fold_get_lines (fold, &start_line, &end_line);
		if (line_number == start_line)
		{
			/* There should be only one fold for line! */
			fold_mark = gtk_source_fold_get_folded (fold) ?
			                    GTK_SOURCE_FOLD_MARK_START_FOLDED :
			                    GTK_SOURCE_FOLD_MARK_START;
			break;
		}
		else if (line_number == end_line)
		{
			fold_mark =  GTK_SOURCE_FOLD_MARK_STOP;
			break;
		}
		else if (line_number > start_line && line_number < end_line)
		{
			++depth;
			fold_mark = GTK_SOURCE_FOLD_MARK_INTERIOR;
		}
		else if (line_number < start_line)
		{
			break;
		}

		folds = g_list_next (folds);
	}

	folds_renderer->priv->fold_mark = fold_mark;
	folds_renderer->priv->depth = depth;
}

static void
gutter_renderer_folds_change_buffer (GtkSourceGutterRenderer *renderer, GtkTextBuffer *buffer)
{
	calculate_size (renderer);
}

static void
gtk_source_gutter_renderer_folds_finalize (GObject *object)
{
	G_OBJECT_CLASS (gtk_source_gutter_renderer_folds_parent_class)->finalize (object);
}

static void
gtk_source_gutter_renderer_folds_class_init (GtkSourceGutterRendererFoldsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkSourceGutterRendererClass *renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	object_class->finalize = gtk_source_gutter_renderer_folds_finalize;

	renderer_class->begin = gutter_renderer_folds_begin;
	renderer_class->draw  = gutter_renderer_folds_draw;

	renderer_class->change_buffer = gutter_renderer_folds_change_buffer;
	renderer_class->query_data = gutter_renderer_folds_query_data;
	renderer_class->activate   = gutter_renderer_folds_activate;

	renderer_class->query_activatable = gutter_renderer_folds_query_activatable;

	g_type_class_add_private (object_class, sizeof (GtkSourceGutterRendererFoldsPrivate));
}

static void
gtk_source_gutter_renderer_folds_init (GtkSourceGutterRendererFolds *renderer)
{
	renderer->priv = G_TYPE_INSTANCE_GET_PRIVATE (renderer, GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS,
	                                              GtkSourceGutterRendererFoldsPrivate);
}

GtkSourceGutterRenderer *
gtk_source_gutter_renderer_folds_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, NULL);
}
