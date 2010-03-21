#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "gtksourcefoldcellrenderer.h"
#include "gtksourceview-typebuiltins.h"

#define MAX_DEPTH	20
#define FIXED_WIDTH	100
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
                                                          guint                       flags);

/***************************************************************************
 *
 *  gtk_source_fold_cell_renderer_class_init:
 *
 *  set up our own get_property and set_property functions, and
 *  override the parent's functions that we need to implement.
 *  And make our new "percentage" property known to the type system.
 *  If you want cells that can be activated on their own (ie. not
 *  just the whole row selected) or cells that are editable, you
 *  will need to override 'activate' and 'start_editing' as well.
 *
 ***************************************************************************/

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
		"mode",	GTK_CELL_RENDERER_MODE_INERT,
		"xpad",	2,
		"ypad",	2,
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
	guint ypad;
	gfloat xalign;
	gfloat yalign;
	gint calc_width;
	gint calc_height;

	g_object_get (G_OBJECT (cell),
		"xpad", &xpad,
		"ypad", &ypad,
		"xalign", &xalign,
		"yalign", &yalign,
		NULL);

	calc_width  = (gint) xpad * 2 + FIXED_WIDTH;
	calc_height = (gint) ypad * 2 + FIXED_HEIGHT;

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

		if (y_offset)
		{
			*y_offset = yalign * (cell_area->height - calc_height);
			*y_offset = MAX (*y_offset, 0);
		}
	}
}

static void
gtk_source_fold_cell_renderer_render (GtkCellRenderer *cell,
                                      GdkWindow       *window,
                                      GtkWidget       *widget,
                                      GdkRectangle    *background_area,
                                      GdkRectangle    *cell_area,
                                      GdkRectangle    *expose_area,
                                      guint            flags)
{
	GtkSourceFoldCellRenderer *cell_fold = GTK_SOURCE_FOLD_CELL_RENDERER (cell);
	GtkStyle                  *style;
	GtkStateType               state;
	gint                       width, height;
	gint                       x_offset, y_offset;


	GdkRectangle visible_rect;
	GdkRectangle line_rect;
	gint win_y;
	gint margin;
	gint c_x, c_y, l, a;
	cairo_t *cr;

	if (cell_fold->priv->fold_mark == GTK_SOURCE_FOLD_MARK_NONE)
	{
		return;
	}

	style = gtk_widget_get_style (widget);
	state = flags;

	cr = gdk_cairo_create (window);
	gdk_cairo_set_source_color (cr, &style->fg[state]);
	cairo_set_line_width (cr, 1);
	//gtk_source_fold_cell_renderer_get_size (cell, widget, cell_area,
	//                                  &x_offset, &y_offset,
	//                                  &width, &height);
	c_x = cell_area->x + cell_area->width / 2;
	c_y = cell_area->y + cell_area->height / 2;
	l   = 5;
	a   = 2;


	switch (cell_fold->priv->fold_mark)
	{
		case GTK_SOURCE_FOLD_MARK_INTERIOR:
			cairo_move_to (cr, c_x + 0.5, cell_area->y + 0.5);
			cairo_rel_line_to (cr, 0, cell_area->height);
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_STOP:
			cairo_move_to (cr, c_x + 0.5, cell_area->y + 0.5);
			cairo_rel_line_to (cr, 0, cell_area->height - l);
			cairo_rel_line_to (cr, cell_area->width / 2,0);
			if (cell_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + .5, c_y + l + .5);
				cairo_line_to (cr, c_x + .5, cell_area->y + cell_area->height);

			}
			cairo_stroke (cr);
			break;

		case GTK_SOURCE_FOLD_MARK_START_FOLDED:
			cairo_move_to (cr, c_x + .5 , c_y - l + a +.5);
			cairo_rel_line_to (cr, 0, 2 * (l - a));

		case GTK_SOURCE_FOLD_MARK_START:
			if (cell_fold->priv->depth > 0)
			{
				cairo_move_to (cr, c_x + .5, cell_area->y + .5);
				cairo_line_to (cr, c_x + .5, c_y + .5 - l);

				cairo_move_to (cr, c_x + .5, c_y + l + .5);
				cairo_line_to (cr, c_x + .5, cell_area->y + cell_area->height);
			}

			if (state == GTK_STATE_PRELIGHT)
			{
				cairo_t *cr = gdk_cairo_create (window);
				gdk_cairo_set_source_color (cr, &style->bg[state]);

				cairo_rectangle (cr, c_x  + .5 - l, c_y - l  + .5, 2 * l, 2 * l);

				cairo_fill (cr);
				cairo_destroy (cr);
			}

			cairo_rectangle (cr, c_x  + .5 - l, c_y - l  + .5, 2 * l, 2 * l);
			cairo_move_to (cr, c_x + a + .5 - l, c_y + 0.5);
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
