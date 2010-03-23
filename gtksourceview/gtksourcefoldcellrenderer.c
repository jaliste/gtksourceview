#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "gtksourcefoldcellrenderer.h"
#include "gtksourceview-typebuiltins.h"

enum
{
	PROP_0,
	PROP_FOLD_MARK,
	PROP_DEPTH,
	PROP_PERCENT_FILL,

	/* Unimplemented */
	PROP_YPAD,
	PROP_XALIGN,
	PROP_YALIGN
};

struct _GtkSourceFoldCellRendererPrivate
{
	GtkSourceFoldMarkType	fold_mark;
	guint			depth;
	gfloat			percent_fill;
};

G_DEFINE_TYPE (GtkSourceFoldCellRenderer, gtk_source_fold_cell_renderer, GTK_TYPE_CELL_RENDERER)

/* Prototypes. */
static void     gtk_source_fold_cell_renderer_get_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             GValue                     *value,
                                                             GParamSpec                 *pspec);

static void     gtk_source_fold_cell_renderer_set_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             const GValue               *value,
                                                             GParamSpec                 *pspec);

static void     gtk_source_fold_cell_renderer_finalize (GObject *gobject);


static void     gtk_source_fold_cell_renderer_get_size   (GtkCellRenderer            *cell,
                                                          GtkWidget                  *widget,
                                                          GdkRectangle               *cell_area,
                                                          gint                       *x_offset,
                                                          gint                       *y_offset,
                                                          gint                       *width,
                                                          gint                       *height);

static void     gtk_source_fold_cell_renderer_render     (GtkCellRenderer            *cell,
                                                          GdkWindow                  *window,
                                                          GtkWidget                  *widget,
                                                          GdkRectangle               *background_area,
                                                          GdkRectangle               *cell_area,
                                                          GdkRectangle               *expose_area,
                                                          GtkCellRendererState        state);

static void
gtk_source_fold_cell_renderer_class_init (GtkSourceFoldCellRendererClass *klass)
{
	GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS (klass);
	GObjectClass         *object_class = G_OBJECT_CLASS (klass);

	gtk_source_fold_cell_renderer_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gtk_source_fold_cell_renderer_finalize;

	object_class->get_property = gtk_source_fold_cell_renderer_get_property;
	object_class->set_property = gtk_source_fold_cell_renderer_set_property;

	cell_class->get_size = gtk_source_fold_cell_renderer_get_size;
	cell_class->render   = gtk_source_fold_cell_renderer_render;

	g_object_class_install_property (object_class,
					 PROP_FOLD_MARK,
					 g_param_spec_enum ("fold-mark",
					 		    _("Fold mark"),
					 		    _("The fold mark to draw"),
					 		    GTK_TYPE_SOURCE_FOLD_MARK_TYPE,
					 		    GTK_SOURCE_FOLD_MARK_NONE,
					 		    G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_DEPTH,
					 g_param_spec_uint ("depth",
							    _("Depth"),
							    _("Depth of the fold, 0 if root fold"),
							    0, G_MAXUINT, 0,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PERCENT_FILL,
					 g_param_spec_float ("percent-fill",
					 		     _("Percent fill"),
					 		     _("The percent to fill for the fold mark"),
					 		     0.0, 1.0, 0.75,
					 		     G_PARAM_READWRITE));

	/* Override the unimplemented properties */
	g_object_class_override_property (object_class,
					  PROP_YPAD,
					  "ypad");
	g_object_class_override_property (object_class,
					  PROP_XALIGN,
					  "xalign");
	g_object_class_override_property (object_class,
					  PROP_YALIGN,
					  "yalign");

	g_type_class_add_private (object_class, sizeof (GtkSourceFoldCellRendererPrivate));
}

static void
gtk_source_fold_cell_renderer_get_property (GObject    *object,
                                            guint       param_id,
                                            GValue     *value,
                                            GParamSpec *psec)
{
	GtkSourceFoldCellRenderer *cell = GTK_SOURCE_FOLD_CELL_RENDERER (object);

	switch (param_id)
	{
		case PROP_FOLD_MARK:
			g_value_set_enum (value,
					  gtk_source_fold_cell_renderer_get_fold_mark (cell));
			break;

		case PROP_DEPTH:
			g_value_set_uint (value,
					  gtk_source_fold_cell_renderer_get_depth (cell));
			break;

		case PROP_PERCENT_FILL:
			g_value_set_float (value,
					   gtk_source_fold_cell_renderer_get_percent_fill (cell));
			break;

		/* Unimplemented */
		case PROP_YPAD:
		case PROP_XALIGN:
		case PROP_YALIGN:
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, psec);
			break;
	}
}

static void
gtk_source_fold_cell_renderer_set_property (GObject      *object,
                                            guint         param_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	GtkSourceFoldCellRenderer *cell = GTK_SOURCE_FOLD_CELL_RENDERER (object);

	switch (param_id)
	{
		case PROP_FOLD_MARK:
			gtk_source_fold_cell_renderer_set_fold_mark (cell,
								     g_value_get_enum (value));
			break;

		case PROP_DEPTH:
			gtk_source_fold_cell_renderer_set_depth (cell,
								 g_value_get_uint (value));
			break;

		case PROP_PERCENT_FILL:
			gtk_source_fold_cell_renderer_set_percent_fill (cell,
									g_value_get_float (value));
			break;

		/* Unimplemented */
		case PROP_YPAD:
		case PROP_XALIGN:
		case PROP_YALIGN:
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
			break;
	}
}

static void
gtk_source_fold_cell_renderer_init (GtkSourceFoldCellRenderer *cell)
{
	cell->priv = G_TYPE_INSTANCE_GET_PRIVATE (cell, GTK_TYPE_SOURCE_FOLD_CELL_RENDERER,
						  GtkSourceFoldCellRendererPrivate);
	g_object_set (G_OBJECT (cell),
		      "xpad", 2,
		      "percent-fill", 0.75,
		      NULL);

}

static void
gtk_source_fold_cell_renderer_finalize (GObject *object)
{
	G_OBJECT_CLASS (gtk_source_fold_cell_renderer_parent_class)->finalize (object);
}

GtkCellRenderer *
gtk_source_fold_cell_renderer_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_FOLD_CELL_RENDERER, NULL);
}

/* This function's contents were taken from gtk+/gtkcellrenderertext.c */
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
gtk_source_fold_cell_renderer_get_size (GtkCellRenderer *cell,
                                        GtkWidget       *widget,
                                        GdkRectangle    *cell_area,
                                        gint            *x_offset,
                                        gint            *y_offset,
                                        gint            *width,
                                        gint            *height)
{
	guint xpad;
	gfloat percent_fill;
	gint calc_width;
	gint calc_height;

	g_object_get (G_OBJECT (cell),
		"xpad", &xpad,
		"percent-fill", &percent_fill,
		NULL);

	calc_height = get_line_height (widget);
	calc_width  = (gint) xpad * 2 + calc_height * percent_fill;

	if (width)
		*width = calc_width;

	if (height)
		*height = calc_height;

	if (x_offset)
		*x_offset = 0;

	if (y_offset)
		*y_offset = 0;
}

static void
gtk_source_fold_cell_renderer_render (GtkCellRenderer      *cell,
                                      GdkWindow            *window,
                                      GtkWidget            *widget,
                                      GdkRectangle         *background_area,
                                      GdkRectangle         *cell_area,
                                      GdkRectangle         *expose_area,
                                      GtkCellRendererState  state)
{
	GtkSourceFoldCellRenderer *cell_fold = GTK_SOURCE_FOLD_CELL_RENDERER (cell);
	GtkStyle                  *style;
	gint                       xpad;
	gfloat                     percent_fill;
	gint                       line_height;
	cairo_t                   *cr;

	struct {
		gint x;
		gint y;
		gint width;
		gint height;
		gint spacing;

		struct {
			gint x;
			gint y;
		} mid;
	} line, mark;

	if (cell_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_NONE)
	{
		return;
	}

	style = gtk_widget_get_style (widget);

	cr = gdk_cairo_create (window);
	gdk_cairo_set_source_color (cr, &style->fg[state]);
	cairo_set_line_width (cr, 1);

	line_height = get_line_height (widget);

        g_object_get (G_OBJECT (cell),
        	"xpad", &xpad,
        	"percent-fill", &percent_fill,
        	NULL);

	line.x       = cell_area->x + xpad;
	line.y       = cell_area->y;
	line.width   = cell_area->width - xpad * 2;
	line.height  = cell_area->height;
	line.mid.x   = line.x + line.width  / 2.0;
	line.mid.y   = line.y + line.height / 2.0;

	mark = line;

	mark.height  = line_height * percent_fill;
	mark.y       = cell_area->y + mark.height * ((1 - percent_fill) / 2);
	mark.mid.y   = mark.y + mark.height / 2.0;
	mark.spacing = mark.width / 5;

	switch (cell_fold->priv->fold_mark)
	{
		case GTK_SOURCE_FOLD_MARK_INTERIOR:
			cairo_move_to (cr, line.mid.x + 0.5, line.y);
			cairo_rel_line_to (cr, 0, line.height);
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_STOP:
			cairo_move_to (cr, line.mid.x + 0.5, line.y);
			cairo_rel_line_to (cr, 0, line.height - line_height / 2.0);
			cairo_rel_line_to (cr, line.width / 2.0, 0);

			if (cell_fold->priv->depth > 0)
			{
				cairo_rel_move_to (cr, -(line.width / 2.0), 0);
				cairo_line_to (cr, line.mid.x + 0.5, line.y + line.height);
			}
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_START_FOLDED:
			cairo_move_to (cr, mark.mid.x + 0.5, mark.y + mark.spacing);
			cairo_rel_line_to (cr, 0, mark.width - mark.spacing * 2);

		case GTK_SOURCE_FOLD_MARK_START:
			cairo_rectangle (cr, mark.x + 0.5, mark.y + 0.5,
					 mark.width, mark.height);

			cairo_move_to (cr, mark.x + mark.spacing + 0.5, mark.mid.y + 0.5);
			cairo_rel_line_to (cr, mark.width - mark.spacing * 2.0, 0);

			if (state == GTK_STATE_PRELIGHT)
			{
				cairo_t *cr = gdk_cairo_create (window);
				gdk_cairo_set_source_color (cr, &style->light[state]);

				cairo_rectangle (cr, mark.x + 0.5, mark.y + 0.5,
						 mark.width, mark.height);

				cairo_fill (cr);
				cairo_destroy (cr);
			}

			if (cell_fold->priv->depth == 0 &&
			    cell_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_START)
			{
				cairo_move_to (cr, line.mid.x + 0.5, mark.y + mark.height + line_height / 2.0 + 0.5);
				cairo_line_to (cr, line.mid.x + 0.5, line.y + line.height);
			}
			else if (cell_fold->priv->depth > 0)
			{
				cairo_move_to (cr, line.mid.x + 0.5, line.y);
				cairo_line_to (cr, line.mid.x + 0.5, mark.y + 0.5);

				cairo_move_to (cr, line.mid.x + 0.5, mark.y + mark.height + 0.5);
				cairo_line_to (cr, line.mid.x + 0.5, line.y + line.height);
			}
			cairo_stroke (cr);
			break;

		default:
			break;
	}
	cairo_destroy (cr);
}

/**
 * gtk_source_fold_cell_renderer_set_fold_mark:
 * @cell: a #GtkSourceFoldCellRenderer.
 * @fold_mark: the desired display among #GtkSourceFoldMarkType.
 *
 * Set the desired display for fold mark.
 **/
void
gtk_source_fold_cell_renderer_set_fold_mark (GtkSourceFoldCellRenderer *cell,
					     GtkSourceFoldMarkType fold_mark)
{
	g_return_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell));

	if (cell->priv->fold_mark == fold_mark)
		return;

	cell->priv->fold_mark = fold_mark;
}

/**
 * gtk_source_fold_cell_renderer_get_fold_mark:
 * @cell: a #GtkSourceFoldCellRenderer.
 *
 * Returns a #GtkSourceFoldMarkType value specifying
 * how the fold will be displayed.
 *
 * Return value: a #GtkSourceFoldMarkType value.
 **/
GtkSourceFoldMarkType
gtk_source_fold_cell_renderer_get_fold_mark (GtkSourceFoldCellRenderer *cell)
{
	g_return_val_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell), GTK_SOURCE_FOLD_MARK_NONE);

	return cell->priv->fold_mark;
}

/**
 * gtk_source_fold_cell_renderer_set_depth:
 * @cell: a #GtkSourceFoldCellRenderer.
 * @depth: the depth of the fold.
 *
 * Set the depth of the fold.
 **/
void
gtk_source_fold_cell_renderer_set_depth (GtkSourceFoldCellRenderer *cell,
					 guint depth)
{
	g_return_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell));

	if (cell->priv->depth == depth)
		return;

	cell->priv->depth = depth;
}

/**
 * gtk_source_fold_cell_renderer_get_depth:
 * @cell: a #GtkSourceFoldCellRenderer.
 *
 * Returns the depth of the fold.
 *
 * Return value: the depth of the fold.
 **/
guint
gtk_source_fold_cell_renderer_get_depth (GtkSourceFoldCellRenderer *cell)
{
	g_return_val_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell), 0);

	return cell->priv->depth;
}

/**
 * gtk_source_fold_cell_renderer_set_percent_fill:
 * @cell: a #GtkSourceFoldCellRenderer.
 * @depth: the percent to fill.
 *
 * Set the percent to fill.
 **/
void
gtk_source_fold_cell_renderer_set_percent_fill (GtkSourceFoldCellRenderer *cell,
						gfloat percent_fill)
{
	g_return_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell));

	if (cell->priv->percent_fill == percent_fill)
		return;

	cell->priv->percent_fill = percent_fill;
}

/**
 * gtk_source_fold_cell_renderer_get_percent_fill:
 * @cell: a #GtkSourceFoldCellRenderer.
 *
 * Returns the percent to fill.
 *
 * Return value: the percent to fill.
 **/
gfloat
gtk_source_fold_cell_renderer_get_percent_fill (GtkSourceFoldCellRenderer *cell)
{
	g_return_val_if_fail (GTK_IS_SOURCE_FOLD_CELL_RENDERER (cell), 0.75);

	return cell->priv->percent_fill;
}
