/* -*- mode: c; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcehighlighter.c
 *
 *  Copyright (C) 2003 - Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *  Copyright (C) 2005, 2006 - Marco Barisione, Emanuele Aina
 *  Copyright (C) 2010 Jose Aliste <jose.aliste@gmail.com>
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

#include "gtksourceview-i18n.h"
#include "gtksourcehighlighter.h"
#include "gtktextregion.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcebuffer.h"
#include "gtksourcestyle-private.h"
#include "gtksourcecontextengine-private.h"

#include <glib.h>

#include <errno.h>
#include <string.h>

#undef ENABLE_DEBUG
#undef ENABLE_PROFILE
#undef ENABLE_CHECK_TREE
#undef ENABLE_MEMORY_DEBUG /* define it to make it print memory usage information */
			   /* it won't work with GRegex */

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

#if defined (ENABLE_DEBUG) || defined (ENABLE_PROFILE) || \
    defined (ENABLE_CHECK_TREE)
#define NEED_DEBUG_ID
#endif
static void
set_tag_style_hash_cb (const char           *style,
		       GSList               *tags,
		       GtkSourceHighlighter *highlighter);




struct _GtkSourceHighlighterPrivate
{
	GtkTextBuffer		*buffer;
	GtkSourceStyleScheme	*style_scheme;

	/* All tags indexed by style name: values are GSList's of tags, ref()'ed. */
	GHashTable		*tags;

	/* pointer to the styles_map of Language. */ 
	GHashTable		*styles_map;

	/* Whether or not to actually highlight the buffer. */
	gboolean		 highlight;

       /* Region covering the unhighlighted text. */
       GtkTextRegion           *refresh_region;

	/* Pointer to the segment tree created by the syntax analyzer */
	Segment			*segment_tree;

#ifdef ENABLE_MEMORY_DEBUG
	guint			 mem_usage_timeout;
#endif
};


static void
unhighlight_region_cb (G_GNUC_UNUSED gpointer style,
		       GSList   *tags,
		       gpointer  user_data)
{
	struct BufAndIters *data = user_data;

	while (tags != NULL)
	{
		gtk_text_buffer_remove_tag (data->buffer,
					    tags->data,
					    data->start,
					    data->end);
		tags = tags->next;
	}
}

static void
unhighlight_region (GtkSourceHighlighter *highlighter,
		    const GtkTextIter      *start,
		    const GtkTextIter      *end)
{
	struct BufAndIters data;

	data.buffer = highlighter->priv->buffer;
	data.start = start;
	data.end = end;

	if (gtk_text_iter_equal (start, end))
		return;

	g_hash_table_foreach (highlighter->priv->tags, (GHFunc) unhighlight_region_cb, &data);
}

#define MAX_STYLE_DEPENDENCY_DEPTH	50

static void
set_tag_style (GtkSourceHighlighter *highlighter,
	       GtkTextTag             *tag,
	       const gchar            *style_id)
{
	printf ("Set Tag style\n");
	GtkSourceStyle *style;

	const char *map_to = style_id;

	int guard = 0;

	g_return_if_fail (GTK_IS_TEXT_TAG (tag));
	g_return_if_fail (style_id != NULL);

	_gtk_source_style_apply (NULL, tag);

	if (highlighter->priv->style_scheme == NULL)
		return;

	style = gtk_source_style_scheme_get_style (highlighter->priv->style_scheme, style_id);

	while (style == NULL)
	{
		GtkSourceStyleInfo *info;

		if (guard > MAX_STYLE_DEPENDENCY_DEPTH)
		{
			g_warning ("Potential circular dependency between styles detected for style '%s'", style_id);
			break;
		}

		++guard;

		/* FIXME Style references really must be fixed, both parser for
		 * sane use in lang files, and highlighter for safe use. */
		info = g_hash_table_lookup (highlighter->priv->styles_map, map_to);

		map_to = (info != NULL) ? info->map_to : NULL;

		if (!map_to)
			break;

		style = gtk_source_style_scheme_get_style (highlighter->priv->style_scheme, map_to);
	}

	/* not having style is fine, since parser checks validity of every style reference,
	 * so we don't need to spit a warning here */
	if (style != NULL)
		_gtk_source_style_apply (style, tag);
}



static void
apply_tags (GtkSourceHighlighter  *highlighter,
	    Segment		*segment,
	    gint		 start_offset,
	    gint		 end_offset)
{
	GtkTextTag *tag;
	GtkTextIter start_iter, end_iter;
	GtkTextBuffer *buffer = highlighter->priv->buffer;
	SubPattern *sp;
	Segment *child;

	g_assert (segment != NULL);

	/* Non-annotated segments are invalid.*/
	if (!segment->annot)
		return;

	if (segment->start_at >= end_offset || segment->end_at <= start_offset)
		return;

	start_offset = MAX (start_offset, segment->start_at);
	end_offset = MIN (end_offset, segment->end_at);

	tag = segment->annot->style_tag; //get_context_tag

	if (tag != NULL)
	{
		//printf("tag is not null\n");
		gint style_start_at, style_end_at;

		style_start_at = start_offset;
		style_end_at = end_offset;

		if (segment->annot->style_inside)
		{
			style_start_at = MAX (segment->start_at + segment->start_len, start_offset);
			style_end_at = MIN (segment->end_at - segment->end_len, end_offset);
		}

		if (style_start_at > style_end_at)
		{
			g_critical ("%s: oops", G_STRLOC);
		}
		else
		{	/* FIXME: We should cache the start_at and end_at so we only apply the tag
			   where is needed, instead of erasing all the highlighting */
			gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, style_start_at);
			end_iter = start_iter;
			gtk_text_iter_forward_chars (&end_iter, style_end_at - style_start_at);
			gtk_text_buffer_apply_tag (highlighter->priv->buffer, tag, &start_iter, &end_iter);
		}
	}

	for (sp = segment->sub_patterns; sp != NULL; sp = sp->next)
	{
		if (sp->start_at >= start_offset && sp->end_at <= end_offset)
		{
			gint start = MAX (start_offset, sp->start_at);
			gint end = MIN (end_offset, sp->end_at);
			
			tag = sp->annot->style_tag;

			if (tag != NULL)
			{
				gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, start);
				end_iter = start_iter;
				gtk_text_iter_forward_chars (&end_iter, end - start);
				gtk_text_buffer_apply_tag (highlighter->priv->buffer, tag, &start_iter, &end_iter);
			}
		}
	}

	for (child = segment->children;
	     child != NULL && child->start_at < end_offset;
	     child = child->next)
	{
		if (child->end_at > start_offset)
			apply_tags (highlighter, child, start_offset, end_offset);
	}
}

/**
 * highlight_region:
 *
 * @highlighter: a #GtkSourceHighlighter.
 * @start: the beginning of the region to highlight.
 * @end: the end of the region to highlight.
 *
 * Highlights the specified region.
 */
static void
highlight_region (GtkSourceHighlighter *highlighter,
		  const GtkTextIter  *start,
		  const GtkTextIter  *end)
{
#ifdef ENABLE_PROFILE
	GTimer *timer;
#endif
	GtkTextIter real_end;

	real_end = *end;
	if (gtk_text_iter_starts_line (&real_end))
		gtk_text_iter_backward_char (&real_end);
	if (gtk_text_iter_compare (start, &real_end) >= 0)
		return;

#ifdef ENABLE_PROFILE
	timer = g_timer_new ();
#endif

	/* First we need to delete tags in the regions. */
	unhighlight_region (highlighter, start, &real_end);

	apply_tags (highlighter, highlighter->priv->segment_tree,
		    gtk_text_iter_get_offset (start),
		    gtk_text_iter_get_offset (&real_end));
	g_hash_table_foreach (highlighter->priv->tags, (GHFunc) set_tag_style_hash_cb, highlighter);

#ifdef ENABLE_PROFILE
	g_print ("highlight (from %d to %d), %g ms elapsed\n",
		 gtk_text_iter_get_offset (start),
		 gtk_text_iter_get_offset (&real_end),
		 g_timer_elapsed (timer, NULL) * 1000);
	g_timer_destroy (timer);
#endif
}


/* FIXME: ensure_highlighted was used before to highlight regions. 
   Since we no longer have refresh_region in the highlightengine, 
   we use highlight_region directly for the moment. I need to investigate more
   before I can add this method back */
/**
 * ensure_highlighted:
 *
 * @highlighter: a #GtkSourceHighlighter.
 * @start: the beginning of the region to highlight.
 * @end: the end of the region to highlight.
 *
 * Updates text tags in reanalyzed parts of given area.
 * It applies tags according to whatever is in the syntax
 * tree currently, so highlighting may not be correct
 * (gtk_source_highlighter_update_highlight is the method
 * that actually ensures correct highlighting).
 */
#if 0 
static void
ensure_highlighted (GtkSourceHighlighter *highlighter,
		    const GtkTextIter      *start,
		    const GtkTextIter      *end,
		    GtkTextRegion	*refresh_region)
{
	GtkTextRegion *region;
	GtkTextRegionIterator reg_iter;

	/* Get the subregions not yet highlighted. */
	region = gtk_text_region_intersect (refresh_region, start, end);

	if (region == NULL)
		return;

	gtk_text_region_get_iterator (region, &reg_iter, 0);

	/* Highlight all subregions from the intersection.
	 * hopefully this will only be one subregion. */
	while (!gtk_text_region_iterator_is_end (&reg_iter))
	{
		GtkTextIter s, e;
		gtk_text_region_iterator_get_subregion (&reg_iter, &s, &e);
		highlight_region (highlighter, &s, &e);
		gtk_text_region_iterator_next (&reg_iter);
	}

	gtk_text_region_destroy (region, TRUE);

	/* Remove the just highlighted region. */
	gtk_text_region_subtract (highlighter->priv->refresh_region, start, end);
}
#endif

/**
 * update_highlight_cb:
 *
 * @highlighter: a #GtkSourceHighlighter.
 * @start: start of area to update.
 * @end: start of area to update.
 * @synchronous: whether it should block until everything
 * is analyzed/highlighted.
 *
 * GtkSourceEngine::update_highlight method.
 *
 * Makes sure the area is analyzed and highlighted. If @synchronous
 * is %FALSE, then it queues idle worker.
 */
static void
update_highlight_cb (GtkSourceHighlighter *highlight_handler,
		     const GtkTextIter  *start,
		     const GtkTextIter  *end,
		     GtkSourceBuffer    *buffer)
{
	if (!highlight_handler->priv->highlight)
		return;
	highlight_region (highlight_handler, start, end);
	
}

/**
 * enable_highlight:
 *
 * @highlighter: a #GtkSourceHighlighter.
 * @enable: whether to enable highlighting.
 *
 * Whether to highlight (i.e. apply tags) analyzed area.
 * Note that this does not turn on/off the analyzis stuff,
 * it affects only text tags.
 */
static void
enable_highlight (GtkSourceHighlighter *highlighter,
		  gboolean                enable)
{
	GtkTextIter start, end;

	if (!enable == !highlighter->priv->highlight)
		return;

	highlighter->priv->highlight = enable != 0;
	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (highlighter->priv->buffer),
				    &start, &end);

	if (enable)
		/*FIXME: here we used to call refresh_range (highlighter, &start, &end, TRUE);
		  but that is in the SyntaxAnalyzer now. */
		highlight_region (highlighter, &start, &end);
	else
		unhighlight_region (highlighter, &start, &end);

}

static void
buffer_notify_highlight_syntax_cb (GtkSourceHighlighter *highlighter)
{
	gboolean highlight;
	g_object_get (highlighter->priv->buffer, "highlight-syntax", &highlight, NULL);
	enable_highlight (highlighter, highlight);
}

G_DEFINE_TYPE (GtkSourceHighlighter, _gtk_source_highlighter, G_TYPE_OBJECT)

/* GtkSourceHighlighter class ------------------------------------------- */


/*
 * gtk_source_highlighter_attach_buffer:
 *
 * @highlighter: #GtkSourceHighlighter.
 * @buffer: buffer.
 *
 * Detaches highlighter from previous buffer, and attaches to @buffer if
 * it's not %NULL. Only called from set_analyzer
 */
void
_gtk_source_highlighter_attach_buffer (GtkSourceHighlighter *highlighter,
				     GtkTextBuffer      *buffer)
{

	//g_return_if_fail (!buffer || GTK_IS_CONTEXT_ (buffer));

	if (highlighter->priv->buffer == buffer)
		return;

	/* Detach previous buffer if there is one. */
	if (highlighter->priv->buffer != NULL)
	{
		g_signal_handlers_disconnect_by_func (highlighter->priv->buffer,
						      (gpointer) buffer_notify_highlight_syntax_cb,
						      highlighter);
		/* TODO: Should I clean the root_segment? */
		/*if (highlighter->priv->root_segment != NULL)
			segment_destroy (highlighter, highlighter->priv->root_segment);
		highlighter->priv->root_segment = NULL;
		*/
		

		/* FIXME:		
		if (highlighter->priv->refresh_region != NULL)
			gtk_text_region_destroy (highlighter->priv->refresh_region, FALSE);
		highlighter->priv->refresh_region = NULL;
		*/
	}

	highlighter->priv->buffer = buffer;

	if (buffer != NULL)
	{
		
		highlighter->priv->tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

		g_object_get (highlighter->priv->buffer, "highlight-syntax", &highlighter->priv->highlight, NULL);
		
		g_signal_connect_swapped (buffer,
					  "notify::highlight-syntax",
					  G_CALLBACK (buffer_notify_highlight_syntax_cb),
					  highlighter);
		g_signal_connect_swapped (buffer, "highlight_updated", 
					  G_CALLBACK (update_highlight_cb),
					  highlighter);
	}
}


static void
set_tag_style_hash_cb (const char         *style,
		       GSList             *tags,
		       GtkSourceHighlighter *highlighter)
{
	while (tags != NULL)
	{
		set_tag_style (highlighter, tags->data, style);
		tags = tags->next;
	}
}

/**
 * gtk_source_highlighter_set_style_scheme:
 *
 * @highlighter: #GtkSourceHighlighter.
 * @scheme: #GtkSourceStyleScheme to set.
 *
 * GtkSourceEngine::set_style_scheme method.
 * Sets current style scheme, updates tag styles and everything.
 */
void
_gtk_source_highlighter_set_style_scheme (GtkSourceHighlighter      *highlighter,
				        GtkSourceStyleScheme 	*scheme)
{
	g_return_if_fail (GTK_IS_SOURCE_HIGHLIGHTER (highlighter));
	g_return_if_fail (GTK_IS_SOURCE_STYLE_SCHEME (scheme) || scheme == NULL);

	if (scheme != highlighter->priv->style_scheme)
	{
		if (highlighter->priv->style_scheme != NULL)
			g_object_unref (highlighter->priv->style_scheme);

		highlighter->priv->style_scheme = scheme ? g_object_ref (scheme) : NULL;
		g_hash_table_foreach (highlighter->priv->tags, (GHFunc) set_tag_style_hash_cb, highlighter);
	}
		 
}

void
_gtk_source_highlighter_set_styles_map (GtkSourceHighlighter    *highlighter,
				      GHashTable	    *styles)
{
	g_return_if_fail (GTK_IS_SOURCE_HIGHLIGHTER (highlighter));

	if (highlighter->priv->styles_map != styles)
		highlighter->priv->styles_map = styles;
}

void
_gtk_source_highlighter_set_analyzer (GtkSourceHighlighter *highlighter,
				      GtkSourceEngine  	   *analyzer)
{
	GtkSourceContextEngine  *engine; 

	g_return_if_fail (GTK_IS_SOURCE_HIGHLIGHTER (highlighter));
	g_return_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (analyzer));

	engine = GTK_SOURCE_CONTEXT_ENGINE (analyzer);

	_gtk_source_highlighter_attach_buffer (highlighter, 
					       _gtk_source_context_engine_get_buffer (engine));
	highlighter->priv->segment_tree = _gtk_source_context_engine_get_tree (engine);
	highlighter->priv->tags = _gtk_source_context_engine_get_style_tags (engine);

}
		

static void
gtk_source_highlighter_finalize (GObject *object)
{
	GtkSourceHighlighter *highlighter = GTK_SOURCE_HIGHLIGHTER (object);

	if (highlighter->priv->buffer != NULL)
	{
		g_critical ("finalizing highlighter with attached buffer");
		/* Disconnect the buffer (if there is one), which destroys almost
		 * everything. */
		//gtk_source_highlighter_attach_buffer (GTK_SOURCE_ENGINE (highlighter), NULL);
	}

	g_assert (!highlighter->priv->tags);
	//g_assert (!highlighter->priv->root_context);
	g_assert (!highlighter->priv->segment_tree);
	
	if (highlighter->priv->style_scheme != NULL)
		g_object_unref (highlighter->priv->style_scheme);

	G_OBJECT_CLASS (_gtk_source_highlighter_parent_class)->finalize (object);
}

static void
_gtk_source_highlighter_class_init (GtkSourceHighlighterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gtk_source_highlighter_finalize;
	g_type_class_add_private (object_class, sizeof (GtkSourceHighlighterPrivate));
}

static void
_gtk_source_highlighter_init (GtkSourceHighlighter *highlighter)
{
	highlighter->priv = G_TYPE_INSTANCE_GET_PRIVATE (highlighter, GTK_TYPE_SOURCE_HIGHLIGHTER,
						GtkSourceHighlighterPrivate);
}

GtkSourceHighlighter * 	
_gtk_source_highlighter_new (void)
{
	GtkSourceHighlighter *highlighter;

	highlighter = g_object_new (GTK_TYPE_SOURCE_HIGHLIGHTER, NULL);
	highlighter->priv->segment_tree = NULL;

	return highlighter;
}

