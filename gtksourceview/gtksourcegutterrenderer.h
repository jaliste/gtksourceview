/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 * gtksourcegutterrenderer.h
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef __GTK_SOURCE_GUTTER_RENDERER_H__
#define __GTK_SOURCE_GUTTER_RENDERER_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define GTK_TYPE_SOURCE_GUTTER_RENDERER               (gtk_source_gutter_renderer_get_type ())
#define GTK_SOURCE_GUTTER_RENDERER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SOURCE_GUTTER_RENDERER, GtkSourceGutterRenderer))
#define GTK_IS_SOURCE_GUTTER_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_GUTTER_RENDERER))
#define GTK_SOURCE_GUTTER_RENDERER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GTK_TYPE_SOURCE_GUTTER_RENDERER, GtkSourceGutterRendererIface))

typedef struct _GtkSourceGutterRenderer                GtkSourceGutterRenderer;
typedef struct _GtkSourceGutterRendererIface           GtkSourceGutterRendererIface;
typedef struct _GtkSourceGutterRendererIface           GtkSourceGutterRendererInterface;

/**
 * GtkSourceGutterRendererState:
 *
 * @GTK_SOURCE_GUTTER_RENDERER_STATE_NORMAL: normal state
 * @GTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR: area in the renderer represents the
 * line on which the insert cursor is currently positioned
 * @GTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT: the mouse pointer is currently
 * over the activatable area of the renderer
 * @GTK_SOURCE_GUTTER_RENDERER_STATE_SELECTED: area in the renderer represents
 * a line in the buffer which contains part of the selection
 **/
typedef enum
{
	GTK_SOURCE_GUTTER_RENDERER_STATE_NORMAL = 0,
	GTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR = 1 << 0,
	GTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT = 1 << 1,
	GTK_SOURCE_GUTTER_RENDERER_STATE_SELECTED = 1 << 2
} GtkSourceGutterRendererState;

struct _GtkSourceGutterRendererIface
{
	GTypeInterface parent;

	void (*begin)               (GtkSourceGutterRenderer     *renderer,
	                             cairo_t                     *cr,
	                             GtkWidget                   *widget,
	                             const GdkRectangle          *background_area,
	                             const GdkRectangle          *cell_area,
	                             GtkTextIter                 *start,
	                             GtkTextIter                 *end);

	void (*draw)                (GtkSourceGutterRenderer      *renderer,
	                             cairo_t                      *cr,
	                             GtkWidget                    *widget,
	                             const GdkRectangle           *background_area,
	                             const GdkRectangle           *cell_area,
	                             GtkTextIter                  *start,
	                             GtkTextIter                  *end,
	                             GtkSourceGutterRendererState  state);

	void (*end)                 (GtkSourceGutterRenderer      *renderer);

	gboolean (*get_activatable) (GtkSourceGutterRenderer      *renderer,
	                             GtkTextIter                  *iter,
	                             const GdkRectangle           *area,
	                             gint                          x,
	                             gint                          y);

	void (*get_size)            (GtkSourceGutterRenderer      *renderer,
	                             cairo_t                      *cr,
	                             GtkWidget                    *widget,
	                             gint                         *width,
	                             gint                         *height);

	/* Signal handler */
	void (*activate)            (GtkSourceGutterRenderer      *renderer,
	                             GtkTextIter                  *iter,
	                             const GdkRectangle           *area,
	                             gint                          x,
	                             gint                          y);

	void (*size_changed)        (GtkSourceGutterRenderer      *renderer);

	void (*queue_draw)          (GtkSourceGutterRenderer      *renderer);

	gboolean (*query_tooltip)   (GtkSourceGutterRenderer      *renderer,
	                             GtkTextIter                  *iter,
	                             const GdkRectangle           *area,
	                             gint                          x,
	                             gint                          y,
	                             GtkTooltip                   *tooltip);
};

GType gtk_source_gutter_renderer_get_type (void) G_GNUC_CONST;

void     gtk_source_gutter_renderer_begin           (GtkSourceGutterRenderer      *renderer,
                                                     cairo_t                      *cr,
                                                     GtkWidget                    *widget,
                                                     const GdkRectangle           *background_area,
                                                     const GdkRectangle           *cell_area,
                                                     GtkTextIter                  *start,
                                                     GtkTextIter                  *end);

void     gtk_source_gutter_renderer_draw            (GtkSourceGutterRenderer      *renderer,
                                                     cairo_t                      *cr,
                                                     GtkWidget                    *widget,
                                                     const GdkRectangle           *background_area,
                                                     const GdkRectangle           *cell_area,
                                                     GtkTextIter                  *start,
                                                     GtkTextIter                  *end,
                                                     GtkSourceGutterRendererState  state);

void     gtk_source_gutter_renderer_end             (GtkSourceGutterRenderer      *renderer);

gboolean gtk_source_gutter_renderer_get_activatable (GtkSourceGutterRenderer      *renderer,
                                                     GtkTextIter                  *iter,
                                                     const GdkRectangle           *area,
                                                     gint                          x,
                                                     gint                          y);

void     gtk_source_gutter_renderer_get_size        (GtkSourceGutterRenderer      *renderer,
                                                     cairo_t                      *cr,
                                                     GtkWidget                    *widget,
                                                     gint                         *width,
                                                     gint                         *height);

/* Emits the 'activate' signal */
void     gtk_source_gutter_renderer_activate        (GtkSourceGutterRenderer      *renderer,
                                                     GtkTextIter                  *iter,
                                                     const GdkRectangle           *area,
                                                     gint                          x,
                                                     gint                          y);

/* Emits the 'size-changed' signal */
void     gtk_source_gutter_renderer_size_changed    (GtkSourceGutterRenderer      *renderer);

/* Emits the 'queue-draw' signal */
void     gtk_source_gutter_renderer_queue_draw      (GtkSourceGutterRenderer      *renderer);

/* Emits the 'query-tooltip' signal */
gboolean gtk_source_gutter_renderer_query_tooltip   (GtkSourceGutterRenderer      *renderer,
                                                     GtkTextIter                  *iter,
                                                     const GdkRectangle           *area,
                                                     gint                          x,
                                                     gint                          y,
                                                     GtkTooltip                   *tooltip);

G_END_DECLS

#endif /* __GTK_SOURCE_GUTTER_RENDERER_H__ */
