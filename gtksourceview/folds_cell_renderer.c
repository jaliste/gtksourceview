#include "folds_cell_renderer.h"

/* Some boring function declarations: GObject type system stuff */

static void     folds_cell_renderer_init       (FoldsCellRenderer      *cellprogress);

static void     folds_cell_renderer_class_init (FoldsCellRendererClass *klass);

static void     folds_cell_renderer_get_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             GValue                     *value,
                                                             GParamSpec                 *pspec);

static void     folds_cell_renderer_set_property  (GObject                    *object,
                                                             guint                       param_id,
                                                             const GValue               *value,
                                                             GParamSpec                 *pspec);

static void     folds_cell_renderer_finalize (GObject *gobject);


/* These functions are the heart of our folds cell renderer: */

static void     folds_cell_renderer_get_size   (GtkCellRenderer            *cell,
                                                          GtkWidget                  *widget,
                                                          GdkRectangle               *cell_area,
                                                          gint                       *x_offset,
                                                          gint                       *y_offset,
                                                          gint                       *width,
                                                          gint                       *height);

static void     folds_cell_renderer_render     (GtkCellRenderer            *cell,
                                                          GdkWindow                  *window,
                                                          GtkWidget                  *widget,
                                                          GdkRectangle               *background_area,
                                                          GdkRectangle               *cell_area,
                                                          GdkRectangle               *expose_area,
                                                          guint                       flags);


enum
{
  PROP_FOLD_MARK = 1,
  PROP_DEPTH    = 2
};

#define MAX_DEPTH 20

static   gpointer parent_class;


/***************************************************************************
 *
 *  folds_cell_renderer_get_type: here we register our type with
 *                                          the GObject type system if we
 *                                          haven't done so yet. Everything
 *                                          else is done in the callbacks.
 *
 ***************************************************************************/

GType
folds_cell_renderer_get_type (void)
{
  static GType cell_type = 0;

  if (cell_type == 0)
  {
    static const GTypeInfo cell_info =
    {
      sizeof (FoldsCellRendererClass),
      NULL,                                                     /* base_init */
      NULL,                                                     /* base_finalize */
      (GClassInitFunc) folds_cell_renderer_class_init,
      NULL,                                                     /* class_finalize */
      NULL,                                                     /* class_data */
      sizeof (FoldsCellRenderer),
      0,                                                        /* n_preallocs */
      (GInstanceInitFunc) folds_cell_renderer_init,
    };

    /* Derive from GtkCellRenderer */
    cell_type = g_type_register_static (GTK_TYPE_CELL_RENDERER,
                                                 "FoldsCellRenderer",
                                                  &cell_info,
                                                  0);
  }

  return cell_type;
}


/***************************************************************************
 *
 *  folds_cell_renderer_init: set some default properties of the
 *                                      parent (GtkCellRenderer).
 *
 ***************************************************************************/

static void
folds_cell_renderer_init (FoldsCellRenderer *cellrenderer)
{
  g_object_set (G_OBJECT (cellrenderer),
  	"mode",	GTK_CELL_RENDERER_MODE_INERT,
  	"xpad",	2,
  	"ypad",	2,
  	NULL);
}


/***************************************************************************
 *
 *  folds_cell_renderer_class_init:
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
folds_cell_renderer_class_init (FoldsCellRendererClass *klass)
{
  GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS (klass);
  GObjectClass         *object_class = G_OBJECT_CLASS (klass);

  parent_class           = g_type_class_peek_parent (klass);
  object_class->finalize = folds_cell_renderer_finalize;

  /* Hook up functions to set and get our
   *   folds cell renderer properties */
  object_class->get_property = folds_cell_renderer_get_property;
  object_class->set_property = folds_cell_renderer_set_property;

  /* Override the two crucial functions that are the heart
   *   of a cell renderer in the parent class */
  cell_class->get_size = folds_cell_renderer_get_size;
  cell_class->render   = folds_cell_renderer_render;

  /* Install our very own properties */
  g_object_class_install_property (object_class,
                                   PROP_FOLD_MARK,
                                   g_param_spec_int ("fold_mark",
                                                        "Fold mark",
                                                         "The fold_mark to draw",
                                                         FOLD_MARK_NULL, FOLD_MARK_START_FOLDED, FOLD_MARK_NULL,
                                                         G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_DEPTH,
                                   g_param_spec_int ("depth",
                                                        "Depth",
                                                         "Depth of the fold, 0 if root fold",
                                                         0, MAX_DEPTH, 0,
                                                         G_PARAM_READWRITE));
}


/***************************************************************************
 *
 *  folds_cell_renderer_finalize: free any resources here
 *
 ***************************************************************************/

static void
folds_cell_renderer_finalize (GObject *object)
{
/*
  FoldsCellRenderer *cellrendererprogress = FOLDS_CELL_RENDERER(object);
*/

  /* Free any dynamically allocated resources here */

  (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}


/***************************************************************************
 *
 *  folds_cell_renderer_get_property: as it says
 *
 ***************************************************************************/

static void
folds_cell_renderer_get_property (GObject    *object,
                                            guint       param_id,
                                            GValue     *value,
                                            GParamSpec *psec)
{
  FoldsCellRenderer  *cell = FOLDS_CELL_RENDERER (object);

  switch (param_id)
  {
    case PROP_FOLD_MARK:
      g_value_set_int (value, cell->fold_mark);
      break;

    case PROP_DEPTH:
      g_value_set_int (value, cell->depth);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, psec);
      break;
  }
}


/***************************************************************************
 *
 *  folds_cell_renderer_set_property: as it says
 *
 ***************************************************************************/

static void
folds_cell_renderer_set_property (GObject      *object,
                                            guint         param_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  FoldsCellRenderer *folds_cell = FOLDS_CELL_RENDERER (object);

  switch (param_id)
  {
    case PROP_FOLD_MARK:
      folds_cell->fold_mark = g_value_get_int (value);
      break;

    case PROP_DEPTH:
      folds_cell->depth = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

/***************************************************************************
 *
 *  folds_cell_renderer_new: return a new cell renderer instance
 *
 ***************************************************************************/

GtkCellRenderer *
folds_cell_renderer_new (void)
{
  return g_object_new (FOLDS_TYPE_CELL_RENDERER, NULL);
}


/***************************************************************************
 *
 *  folds_cell_renderer_get_size: crucial - calculate the size
 *                                          of our cell, taking into account
 *                                          padding and alignment properties
 *                                          of parent.
 *
 ***************************************************************************/

#define FIXED_WIDTH   100
#define FIXED_HEIGHT  10

static void
folds_cell_renderer_get_size (GtkCellRenderer *cell,
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
  	"xpad",		&xpad,
  	"ypad",		&ypad,
  	"xalign",	&xalign,
  	"yalign",	&yalign,
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


/***************************************************************************
 *
 *  folds_cell_renderer_render: crucial - do the rendering.
 *
 ***************************************************************************/

static void
folds_cell_renderer_render (GtkCellRenderer *cell,
                                      GdkWindow       *window,
                                      GtkWidget       *widget,
                                      GdkRectangle    *background_area,
                                      GdkRectangle    *cell_area,
                                      GdkRectangle    *expose_area,
                                      guint            flags)
{
	FoldsCellRenderer *cell_fold = FOLDS_CELL_RENDERER (cell);
	GtkStateType                state;
  	gint                        width, height;
  	gint                        x_offset, y_offset;


   	GdkRectangle visible_rect;
	GdkRectangle line_rect;
	gint win_y;
	gint margin;
	gint c_x, c_y, l, a;
	cairo_t *cr;
	if (cell_fold->fold_mark == FOLD_MARK_NULL)
	{
		return;
	}
	cr = gdk_cairo_create (window);
	// gdk_cairo_set_source_color (cr, (GdkColor *)color);
	cairo_set_line_width (cr, 1);
	//folds_cell_renderer_get_size (cell, widget, cell_area,
        //                                  &x_offset, &y_offset,
         //                                 &width, &height);
	c_x = cell_area->x + cell_area->width/2;
	c_y = cell_area->y + cell_area->height/2;
	l   = 5;
	a   = 2;


	switch (cell_fold->fold_mark)
	{
		case FOLD_MARK_INTERIOR:
			cairo_move_to (cr, c_x + 0.5, cell_area->y + 0.5);
                	cairo_rel_line_to (cr, 0, cell_area->height);
			cairo_stroke (cr);
			break;

		case FOLD_MARK_STOP:
			cairo_move_to (cr, c_x + 0.5, cell_area->y + 0.5);
			cairo_rel_line_to (cr, 0, cell_area->height - l);
			cairo_rel_line_to (cr, cell_area->width/2,0);
			if (cell_fold->depth > 0)
			{
				cairo_move_to (cr, c_x + .5, c_y + l + .5);
			    cairo_line_to (cr, c_x + .5, cell_area->y + cell_area->height);

			}
			cairo_stroke (cr);
			break;

		case FOLD_MARK_START_FOLDED:
			cairo_move_to (cr, c_x + .5 , c_y - l + a +.5);
			cairo_rel_line_to (cr, 0, 2*(l - a));

		case FOLD_MARK_START:
			if (cell_fold->depth > 0)
			{
			    cairo_move_to (cr, c_x + .5, cell_area->y + .5);
			    cairo_line_to (cr, c_x + .5, c_y + .5 - l);

			    cairo_move_to (cr, c_x + .5, c_y + l + .5);
			    cairo_line_to (cr, c_x + .5, cell_area->y + cell_area->height);
			}

			cairo_rectangle (cr, c_x  + .5 - l, c_y - l  + .5, 2*l, 2*l);
			cairo_move_to (cr, c_x + a + .5 - l, c_y + 0.5);
			cairo_rel_line_to (cr, 2 * (l - a),0);
			cairo_stroke (cr);
			break;

		default:
			break;
	}
	cairo_destroy (cr);

}
