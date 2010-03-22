#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "gtksourcefoldcellrenderer.h"
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

struct _GtkSourceFoldCellRendererPrivate
{
	GtkSourceFoldMarkType	fold_mark;
	guint			depth;
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

	/* Prevent setting the ypad and yalign, causes incorrect appearence. */
	g_object_class_override_property (object_class,
					  -1,
					  "ypad");
	g_object_class_override_property (object_class,
					  -2,
					  "yalign");

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
	gfloat xalign;
	gint calc_width;
	gint calc_height;

	/* Note: we ignore the ypad and yalign because it
	 *       would make the fold line have gaps and the
	 *       fold mark would not be aligned with the text.
	 */

	g_object_get (G_OBJECT (cell),
		"xpad", &xpad,
		"xalign", &xalign,
		NULL);

	calc_width  = (gint) xpad * 2 + FIXED_WIDTH;
	calc_height = (gint) FIXED_HEIGHT;

	if (width)
		*width = calc_width;

	if (height)
		*height = calc_height;

	if (cell_area)
	{
		if (x_offset)
		{
			*x_offset = xalign * (cell_area->width - calc_width);
			*x_offset = MAX (*x_offset, 0);
		}
	}

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
	gint                       width, height;
	gint                       x_offset, y_offset;
	gint                       xpad;
	gint                       line_height;

	gint c_x, c_y, l, a;
	cairo_t *cr;

	if (cell_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_NONE)
	{
		return;
	}

	style = gtk_widget_get_style (widget);

	cr = gdk_cairo_create (window);
	gdk_cairo_set_source_color (cr, &style->fg[state]);
	cairo_set_line_width (cr, 1);

	/* Note: we ignore the ypad and yalign because it
	 *       would make the fold line have gaps and the
	 *       fold mark would not be aligned with the text.
	 */

	gtk_source_fold_cell_renderer_get_size (cell, widget, cell_area,
						&x_offset, &y_offset,
						&width, &height);

        g_object_get (G_OBJECT (cell),
        	"xpad", &xpad,
        	NULL);

	cell_area->x += x_offset + xpad;
	cell_area->width -= xpad * 2;

	c_x = cell_area->x + cell_area->width / 2;
	c_y = cell_area->y + cell_area->height / 2;
	l   = 5;
	a   = 2;

	switch (cell_fold->priv->fold_mark)
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

			if (cell_fold->priv->depth > 0)
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
			if (cell_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + 0.5, cell_area->y);
				cairo_line_to (cr, c_x + 0.5, c_y + 0.5 - l);

				cairo_move_to (cr, c_x + 0.5, c_y + l + 0.5);
				cairo_line_to (cr, c_x + 0.5, cell_area->y + cell_area->height);
			}

			if (state == GTK_STATE_PRELIGHT)
			{
				cairo_t *cr = gdk_cairo_create (window);
				gdk_cairo_set_source_color (cr, &style->light[state]);

				cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);

				cairo_fill (cr);
				cairo_destroy (cr);
			}

			cairo_rectangle (cr, c_x  + 0.5 - l, c_y - l  + 0.5, 2 * l, 2 * l);
			cairo_move_to (cr, c_x + a + 0.5 - l, c_y + 0.5);
			cairo_rel_line_to (cr, 2 * (l - a), 0);
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
