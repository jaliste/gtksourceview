#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "gtksourcegutterrendererfolds.h"
#include "gtksourceview-typebuiltins.h"

#define MAX_DEPTH	20
#define FIXED_WIDTH	10
#define FIXED_HEIGHT	10

enum
{
	PROP_0,
	PROP_FOLD_MARK,
	PROP_DEPTH
};

struct _GtkSourceGutterRendererFoldsPrivate
{
	GtkSourceFoldMarkType	fold_mark;
	guint			depth;
	GList		       *folds_visible;
};

G_DEFINE_TYPE (GtkSourceGutterRendererFolds, gtk_source_gutter_renderer_folds, GTK_TYPE_SOURCE_GUTTER_RENDERER)

/* Prototypes. */
static void     gtk_source_gutter_renderer_folds_get_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             GValue                     *value,
                                                             GParamSpec                 *pspec);

static void     gtk_source_gutter_renderer_folds_set_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             const GValue               *value,
                                                             GParamSpec                 *pspec);

static void     gtk_source_gutter_renderer_folds_finalize (GObject *gobject);


static void     gtk_source_gutter_renderer_folds_get_size   (GtkSourceGutterRenderer            *renderer,
                                                          gint                       *width,
                                                          gint                       *height);

static void
gutter_renderer_query_data (GtkSourceGutterRenderer      *renderer,
                            GtkTextIter                  *start,
                            GtkTextIter                  *end,
                            GtkSourceGutterRendererState  state);
static void
gtk_source_gutter_renderer_folds_draw (GtkSourceGutterRenderer *renderer,
                                       cairo_t                 *cr,
                                       const GdkRectangle      *background_area,
                                       const GdkRectangle      *cell_area,
				       GtkTextIter	       *start,
				       GtkTextIter	       *end,
                                       GtkSourceGutterRendererState state);

static void
gutter_renderer_folds_begin (GtkSourceGutterRenderer      *renderer,
                            cairo_t                      *cr,
                            const GdkRectangle           *background_area,
                            const GdkRectangle           *cell_area,
                            GtkTextIter                  *start,
                            GtkTextIter                  *end)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkTextBuffer *buffer;
	GList	      *list;
	GtkTextIter    fold_start, fold_end;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);
	buffer = gtk_text_view_get_buffer (gtk_source_gutter_renderer_get_view (renderer));
	folds_renderer->priv->folds_visible = _gtk_source_buffer_get_folds_in_region (GTK_SOURCE_BUFFER (buffer), start, end);
}




static gboolean
gutter_renderer_activate (GtkSourceGutterRenderer *renderer,
                          GtkTextIter             *iter,
                          const GdkRectangle      *area,
                          GdkEvent                *event)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkSourceFold *fold;
	GtkWidget *fold_label = NULL;
	GList *folds;
	GtkTextBuffer *buffer;
	gint start_line;
	gint end_line;
	gint line_number;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);

	folds = folds_renderer->priv->folds_visible;
	buffer = gtk_text_view_get_buffer (gtk_source_gutter_renderer_get_view (renderer));

	line_number = gtk_text_iter_get_line (iter);
	
	while (folds != NULL)
	{
		fold = folds->data;

		gtk_source_fold_get_lines (fold, buffer, &start_line, &end_line);

		if (line_number == start_line && fold != NULL)
		{
			gtk_source_fold_set_folded (fold, !gtk_source_fold_get_folded (fold));
			break;
		}
		folds = g_list_next (folds);
	}
}


			// Add or update the fold label. 
		//	fold_label = g_hash_table_lookup (view->priv->fold_labels,
		//					  fold);

//			if (fold_label == NULL && fold->folded)
//			{
		//		fold_label = _gtk_source_fold_label_new (view);

		//		g_hash_table_insert (view->priv->fold_labels,
		//				     fold, fold_label);

		//		gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (view),
		//						   fold_label,
		//						   GTK_TEXT_WINDOW_TEXT,
		//						   0,
		//						   0);

		//		move_fold_label (GTK_TEXT_VIEW (view), fold, fold_label);
			//}
			// Hide the label if the fold has expanded. 
//			else if (fold_label != NULL && !fold->folded &&
//				 gtk_widget_get_visible (fold_label))
//			{
//				gtk_widget_hide (fold_label);
//			}
//			break;
//
//		}
///
//		folds = g_list_next (folds);
//	}
//}/




static gboolean
gutter_renderer_query_activatable (GtkSourceGutterRenderer *renderer,
                                   GtkTextIter             *iter,
                                   const GdkRectangle      *area,
                                   GdkEvent                *event)
{
	GtkSourceGutterRendererFolds *folds_renderer;
	GtkSourceFold *fold;
	GtkWidget *fold_label = NULL;
	GList *folds;
	GtkTextBuffer *buffer;
	gint start_line;
	gint end_line;
	gint line_number;

	folds_renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);

	folds = folds_renderer->priv->folds_visible;
	buffer = gtk_text_view_get_buffer (gtk_source_gutter_renderer_get_view(renderer));
	line_number = gtk_text_iter_get_line (iter);

	
	while (folds != NULL)
	{
		fold = folds->data;

		gtk_source_fold_get_lines (fold, buffer, &start_line, &end_line);

		if (line_number == start_line && fold != NULL)
		{
			return TRUE;
		}
		folds = g_list_next (folds);
	}
	return FALSE;
}

static void
gtk_source_gutter_renderer_folds_class_init (GtkSourceGutterRendererFoldsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkSourceGutterRendererClass *renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	object_class->finalize = gtk_source_gutter_renderer_folds_finalize;

	object_class->get_property = gtk_source_gutter_renderer_folds_get_property;
	object_class->set_property = gtk_source_gutter_renderer_folds_set_property;

	renderer_class->begin = gutter_renderer_folds_begin;
	renderer_class->draw = gtk_source_gutter_renderer_folds_draw;
	//renderer_class->end = gtk_source_gutter_renderer_folds_end;
	
	renderer_class->get_size = gtk_source_gutter_renderer_folds_get_size;
	//renderer_class->size_changed = gtk_source_gutter_renderer_folds_size_changed;

	renderer_class->query_data = gutter_renderer_query_data;
	renderer_class->query_activatable = gutter_renderer_query_activatable;
	renderer_class->activate = gutter_renderer_activate;

	g_object_class_install_property (object_class,
					 PROP_FOLD_MARK,
					 g_param_spec_enum ("fold_mark",
					 		    _("Fold mark"),
					 		    _("The fold mark to draw"),
					 		    GTK_TYPE_SOURCE_FOLD_MARK_TYPE,
					 		    GTK_SOURCE_FOLD_MARK_NONE,
					 		    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_DEPTH,
					 g_param_spec_uint ("depth",
							    "Depth",
							    "Depth of the fold, 0 if root fold",
							    0, MAX_DEPTH, 0,
							    G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GtkSourceGutterRendererFoldsPrivate));
}

static void
gtk_source_gutter_renderer_folds_get_property (GObject    *object,
                                            guint       param_id,
                                            GValue     *value,
                                            GParamSpec *psec)
{
	GtkSourceGutterRendererFolds *renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (object);

	switch (param_id)
	{
		case PROP_FOLD_MARK:
			g_value_set_enum (value,
					  gtk_source_gutter_renderer_folds_get_fold_mark (renderer));
		break;

		case PROP_DEPTH:
			g_value_set_uint (value,
					  gtk_source_gutter_renderer_folds_get_depth (renderer));
		break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, psec);
		break;
	}
}

static void
gtk_source_gutter_renderer_folds_set_property (GObject      *object,
                                            guint         param_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	GtkSourceGutterRendererFolds *renderer = GTK_SOURCE_GUTTER_RENDERER_FOLDS (object);

	switch (param_id)
	{
		case PROP_FOLD_MARK:
			gtk_source_gutter_renderer_folds_set_fold_mark (renderer,
								     g_value_get_enum (value));
		break;

		case PROP_DEPTH:
			gtk_source_gutter_renderer_folds_set_depth (renderer,
								 g_value_get_uint (value));
		break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gtk_source_gutter_renderer_folds_init (GtkSourceGutterRendererFolds *renderer)
{
	renderer->priv = G_TYPE_INSTANCE_GET_PRIVATE (renderer, GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS,
						  GtkSourceGutterRendererFoldsPrivate);
	g_object_set (G_OBJECT (renderer),
		      "xpad", 2,
		      NULL);

}

static void
gtk_source_gutter_renderer_folds_finalize (GObject *object)
{
	G_OBJECT_CLASS (gtk_source_gutter_renderer_folds_parent_class)->finalize (object);
}

GtkCellRenderer *
gtk_source_gutter_renderer_folds_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, NULL);
}

/* This function's contents was taken from gtk+/gtkcellrenderertext.c */
static gint
get_line_height (GtkWidget	*widget)
{
	PangoContext     *context;
	PangoFontMetrics *metrics;
	gint              line_height;

	context = gtk_widget_get_pango_context (widget);
	metrics = pango_context_get_metrics (context,
					     gtk_widget_get_style (widget)->font_desc,
					     pango_context_get_language (context));
	line_height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
				    pango_font_metrics_get_descent (metrics));
	pango_font_metrics_unref (metrics);

	return line_height;
}

static void
gtk_source_gutter_renderer_folds_get_size (GtkSourceGutterRenderer *renderer,
                                        gint            *width,
                                        gint            *height)
{
	guint xpad;
	gfloat xalign;
	gint calc_width;
	gint calc_height;

	/* Note: we ignore the ypad and yalign because it
	 *       would make the fold line have gaps and the
	 *       fold mark would not be aligned with the text.
	 */

	g_object_get (G_OBJECT (renderer),
		"xpad", &xpad,
		"xalign", &xalign,
		NULL);

	calc_width  = (gint) xpad * 2 + FIXED_WIDTH;
	calc_height = (gint) FIXED_HEIGHT;

	if (width)
		*width = calc_width;

	if (height)
		*height = calc_height;


/*	if (y_offset)
		*y_offset = 0;
*/

}

static void
gtk_source_gutter_renderer_folds_draw (GtkSourceGutterRenderer *renderer,
                                       cairo_t                 *cr,
                                       const GdkRectangle      *background_area,
                                       const GdkRectangle      *cell_area,
				       GtkTextIter	       *start,
				       GtkTextIter	       *end,
                                       GtkSourceGutterRendererState state)
{
	GtkSourceGutterRendererFolds *renderer_fold = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer);
	GtkStyle                  *style;
	gint                       width, height;
	gint                       x_offset, y_offset;
	gint                       xpad;
	gint                       line_height;
	GtkTextView		  *view;
	GtkTextBuffer		  *buffer;

	gint c_x, c_y, l, a;

		GTK_SOURCE_GUTTER_RENDERER_CLASS (
		gtk_source_gutter_renderer_folds_parent_class)->draw (renderer,
		                                                     cr,
		                                                     background_area,
		                                                     cell_area,
		                                                     start,
		                                                     end,
		                                                     state);

	if (renderer_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_NONE)
	{
		return;
	}
	
	view = gtk_source_gutter_renderer_get_view (renderer);
	style = gtk_widget_get_style (GTK_WIDGET (view));
	buffer = gtk_text_view_get_buffer (view);

	//gdk_cairo_set_source_color (cr, &style->fg[state]);
	cairo_set_line_width (cr, 1);

	/* Note: we ignore the ypad and yalign because it
	 *       would make the fold line have gaps and the
	 *       fold mark would not be aligned with the text.
	 */

	//line_height = get_line_height (widget);
	line_height = 0;
	gtk_source_gutter_renderer_folds_get_size (renderer
					,	
						&width, &height);

        g_object_get (G_OBJECT (renderer),
        	"xpad", &xpad,
        	NULL);

//	cell_area->x += x_offset + xpad;
//	cell_area->width -= xpad * 2;

	c_x = cell_area->x + cell_area->width / 2;
	c_y = cell_area->y + cell_area->height / 2;
	l   = 5;
	a   = 2;
	if (state & GTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT) 
		printf("PRELIT %d\n",gtk_text_iter_get_line(start));

	switch (renderer_fold->priv->fold_mark)
	{
		case GTK_SOURCE_FOLD_MARK_INTERIOR:
			cairo_move_to (cr, c_x + 0.5, cell_area->y);
			cairo_rel_line_to (cr, 0, cell_area->height);
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_STOP:
			cairo_move_to (cr, c_x + 0.5, cell_area->y);
			cairo_rel_line_to (cr, 0, cell_area->height - line_height / 2.0);
			cairo_rel_line_to (cr, cell_area->width / 2.0, 0);

			if (renderer_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + 0.5, c_y);
				cairo_line_to (cr, c_x + 0.5, cell_area->y + cell_area->height);
			}
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_START_FOLDED:
			cairo_move_to (cr, c_x + 0.5 , c_y - l + a + 0.5);
			cairo_rel_line_to (cr, 0, 2 * (l - a));

		case GTK_SOURCE_FOLD_MARK_START:
			if (renderer_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + 0.5, cell_area->y);
				cairo_line_to (cr, c_x + 0.5, c_y + 0.5 - l);

				cairo_move_to (cr, c_x + 0.5, c_y + l + 0.5);
				cairo_line_to (cr, c_x + 0.5, cell_area->y + cell_area->height);
			}


			if (state & GTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT)
			{
				
//				cairo_t *cr = gdk_cairo_create (window);
				cairo_save (cr);				
				gdk_cairo_set_source_color (cr, &style->light[state]);
				cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);
				cairo_fill (cr);
				cairo_restore (cr);
			}

			cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);
			cairo_move_to (cr, c_x + a + 0.5 - l, c_y + 0.5);
			cairo_rel_line_to (cr, 2 * (l - a), 0);
			cairo_stroke (cr);
			break;

		default:
			break;
	}
}


static void
gutter_renderer_query_data (GtkSourceGutterRenderer      *renderer,
                            GtkTextIter                  *start,
                            GtkTextIter                  *end,
                            GtkSourceGutterRendererState  state)
{
	GtkSourceFold *fold;
	GList *folds;
	GtkTextBuffer *buffer;
	GtkSourceFoldMarkType fold_mark;
	gint start_line;
	gint end_line;
	gint line_number;
	gboolean current_line;
	guint depth;
	GtkCellRendererMode mode;
	GtkTextView *view;

	line_number = gtk_text_iter_get_line (start);
	view = gtk_source_gutter_renderer_get_view (renderer);

	current_line = (state & GTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR) &&
	               gtk_text_view_get_cursor_visible (view);

	buffer = gtk_text_view_get_buffer (view);

	if (buffer == NULL)
	{
		return;
	}

	folds = GTK_SOURCE_GUTTER_RENDERER_FOLDS (renderer)->priv->folds_visible;
	//buffer = GTK_TEXT_BUFFER (view->priv->source_buffer);
	fold_mark = GTK_SOURCE_FOLD_MARK_NONE;
	depth = 0;
	mode = GTK_CELL_RENDERER_MODE_INERT;

	while (folds != NULL)
	{
		fold = folds->data;

		gtk_source_fold_get_lines (fold, buffer, &start_line, &end_line);
		if (line_number == start_line)
		{
			// There should be only one fold for line!
			if (gtk_source_fold_get_folded (fold))
			{
				fold_mark =  GTK_SOURCE_FOLD_MARK_START_FOLDED;
			}
			else
			{
				fold_mark =  GTK_SOURCE_FOLD_MARK_START;
			}
			mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
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

	g_object_set (G_OBJECT (renderer),
	              "fold_mark", fold_mark,
	              "depth", depth,
		      "xpad", 2,
		      "xalign", 0.5,
	           //   "mode", mode,
	              NULL);
}
/**
 * gtk_source_gutter_renderer_folds_set_fold_mark:
 * @renderer: a #GtkSourceGutterRendererFolds.
 * @fold_mark: the desired display among #GtkSourceFoldMarkType.
 *
 * Set the desired display for fold mark.
 **/
void
gtk_source_gutter_renderer_folds_set_fold_mark (GtkSourceGutterRendererFolds *renderer,
					     GtkSourceFoldMarkType fold_mark)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS (renderer));

	if (renderer->priv->fold_mark == fold_mark)
		return;

	renderer->priv->fold_mark = fold_mark;
}

/**
 * gtk_source_gutter_renderer_folds_get_fold_mark:
 * @renderer: a #GtkSourceGutterRendererFolds.
 *
 * Returns a #GtkSourceFoldMarkType value specifying
 * how the fold will be displayed.
 *
 * Return value: a #GtkSourceFoldMarkType value.
 **/
GtkSourceFoldMarkType
gtk_source_gutter_renderer_folds_get_fold_mark (GtkSourceGutterRendererFolds *renderer)
{
	g_return_val_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS (renderer), GTK_SOURCE_FOLD_MARK_NONE);

	return renderer->priv->fold_mark;
}

/**
 * gtk_source_gutter_renderer_folds_set_depth:
 * @renderer: a #GtkSourceGutterRendererFolds.
 * @depth: the depth of the fold.
 *
 * Set the depth of the fold.
 **/
void
gtk_source_gutter_renderer_folds_set_depth (GtkSourceGutterRendererFolds *renderer,
					 guint depth)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS (renderer));

	if (renderer->priv->depth == depth)
		return;

	renderer->priv->depth = depth;
}

/**
 * gtk_source_gutter_renderer_folds_get_depth:
 * @renderer: a #GtkSourceGutterRendererFolds.
 *
 * Returns the depth of the fold.
 *
 * Return value: the depth of the fold.
 **/
guint
gtk_source_gutter_renderer_folds_get_depth (GtkSourceGutterRendererFolds *renderer)
{
	g_return_val_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS (renderer), 0);

	return renderer->priv->depth;
}



