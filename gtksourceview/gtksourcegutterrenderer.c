#include "gtksourcegutterrenderer.h"
#include "gtksourceview-marshal.h"

G_DEFINE_INTERFACE (GtkSourceGutterRenderer, gtk_source_gutter_renderer, G_TYPE_INVALID)

enum
{
	ACTIVATE,
	SIZE_CHANGED,
	QUEUE_DRAW,
	QUERY_TOOLTIP,
	QUERY_ACTIVATABLE,
	NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = {0,};

/* Default implementation */
static void
gtk_source_gutter_renderer_begin_default (GtkSourceGutterRenderer *renderer,
                                          cairo_t                 *cr,
                                          GtkWidget               *widget,
                                          const GdkRectangle      *background_area,
                                          const GdkRectangle      *cell_area,
                                          GtkTextIter             *start,
                                          GtkTextIter             *end)
{
	// NOOP
}

static void
gtk_source_gutter_renderer_draw_default (GtkSourceGutterRenderer      *renderer,
                                         cairo_t                      *cr,
                                         GtkWidget                    *widget,
                                         const GdkRectangle           *background_area,
                                         const GdkRectangle           *cell_area,
                                         GtkTextIter                  *start,
                                         GtkTextIter                  *end,
                                         GtkSourceGutterRendererState  state)
{
	// NOOP
}

static void
gtk_source_gutter_renderer_end_default (GtkSourceGutterRenderer *renderer)
{
	// NOOP
}

static void
gtk_source_gutter_renderer_get_size_default (GtkSourceGutterRenderer *renderer,
                                             cairo_t                 *cr,
                                             GtkWidget               *widget,
                                             gint                    *width,
                                             gint                    *height)
{
	g_warning ("The gutter renderer `%s' does not implement `get_size'",
	           g_type_name (G_TYPE_FROM_INSTANCE (renderer)));
}

static void
gtk_source_gutter_renderer_default_init (GtkSourceGutterRendererInterface *iface)
{
	static gboolean initialized = FALSE;

	iface->begin = gtk_source_gutter_renderer_begin_default;
	iface->end = gtk_source_gutter_renderer_end_default;
	iface->draw = gtk_source_gutter_renderer_draw_default;
	iface->get_size = gtk_source_gutter_renderer_get_size_default;

	if (!initialized)
	{
		/**
		 * GtkSourceGutterRenderer::activate:
		 * @renderer: the #GtkSourceGutterRenderer who emits the signal
		 * @iter: a #GtkTextIter
		 * @area: a #GdkRectangle
		 * @x: the x location (in window coordinates)
		 * @y: the y location (in window coordinates)
		 *
		 * The ::activate signal is emitted when the renderer is
		 * activated.
		 *
		 */
		signals[ACTIVATE] =
			g_signal_new ("activate",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, activate),
			              NULL,
			              NULL,
			              _gtksourceview_marshal_VOID__BOXED_BOXED_INT_INT,
			              G_TYPE_NONE,
			              4,
			              GTK_TYPE_TEXT_ITER,
			              GDK_TYPE_RECTANGLE,
			              G_TYPE_INT,
			              G_TYPE_INT);

		/**
		 * GtkSourceGutterRenderer::size-changed:
		 * @renderer: the #GtkSourceGutterRenderer who emits the signal
		 *
		 * The ::size-changed signal is emitted when the renderer has
		 * changed its size. Use #gtk_source_gutter_renderer_size_changed
		 * to emit this signal from an implementation of the
		 * #GtkSourceGutterRenderer interface.
		 *
		 */
		signals[SIZE_CHANGED] =
			g_signal_new ("size-changed",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, size_changed),
			              NULL,
			              NULL,
			              g_cclosure_marshal_VOID__VOID,
			              G_TYPE_NONE,
			              0);

		/**
		 * GtkSourceGutterRenderer::queue-draw:
		 * @renderer: the #GtkSourceGutterRenderer who emits the signal
		 *
		 * The ::queue-draw signal is emitted when the renderer needs
		 * to be redrawn. Use #gtk_source_gutter_renderer_queue_draw
		 * to emit this signal from an implementation of the
		 * #GtkSourceGutterRenderer interface.
		 *
		 */
		signals[QUEUE_DRAW] =
			g_signal_new ("queue-draw",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, queue_draw),
			              NULL,
			              NULL,
			              g_cclosure_marshal_VOID__VOID,
			              G_TYPE_NONE,
			              0);

		signals[QUERY_TOOLTIP] =
			g_signal_new ("query-tooltip",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, query_tooltip),
			              g_signal_accumulator_true_handled,
			              NULL,
			              _gtksourceview_marshal_BOOLEAN__BOXED_BOXED_INT_INT_OBJECT,
			              G_TYPE_BOOLEAN,
			              5,
			              GTK_TYPE_TEXT_ITER,
			              GDK_TYPE_RECTANGLE,
			              G_TYPE_INT,
			              G_TYPE_INT,
			              GTK_TYPE_TOOLTIP);

		/**
		 * GtkSourceGutterRenderer::query-data:
		 * @renderer: the #GtkSourceGutterRenderer who emits the signal
		 * @start: a #GtkTextIter
		 * @end: a #GtkTextIter
		 * @state: the renderer state
		 *
		 * The ::query-data signal is emitted when the renderer needs
		 * to be filled with data just before a cell is drawn. This can
		 * be used by general renderer implementations to allow render
		 * data to be filled in externally.
		 *
		 */
		signals[QUERY_DATA] =
			g_signal_new ("query-data",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, query_data),
			              NULL,
			              NULL,
			              _gtksourceview_marshal_VOID__BOXED_BOXED_FLAGS,
			              G_TYPE_NONE,
			              3,
			              GTK_TYPE_TEXT_ITER,
			              GTK_TYPE_TEXT_ITER,
			              GTK_TYPE_SOURCE_GUTTER_RENDERER_STATE);

		/**
		 * GtkSourceGutterRenderer::query-activatable:
		 * @renderer: the #GtkSourceGutterRenderer who emits the signal
		 * @iter: a #GtkTextIter
		 * @area: a #GdkRectangle
		 * @event: the #GdkEvent that is causing the activatable query
		 *
		 * The ::query-activatable signal is emitted when the renderer
		 * can possibly be activated.
		 *
		 */
		signals[QUERY_ACTIVATABLE] =
			g_signal_new ("query-activatable",
			              G_TYPE_FROM_INTERFACE (iface),
			              G_SIGNAL_RUN_LAST,
			              G_STRUCT_OFFSET (GtkSourceGutterRendererIface, query_activatable),
			              g_signal_accumulator_true_handled,
			              NULL,
			              _gtksourceview_marshal_BOOLEAN__BOXED_BOXED_BOXED,
			              G_TYPE_BOOLEAN,
			              3,
			              GTK_TYPE_TEXT_ITER,
			              GDK_TYPE_RECTANGLE,
			              GDK_TYPE_EVENT);

		initialized = TRUE;
	}
}

/**
 * gtk_source_gutter_renderer_begin:
 * @renderer: a #GtkSourceGutterRenderer
 * @cr: a #cairo_t
 * @widget: a #GtkWidget
 * @background_area: a #GdkRectangle
 * @cell_area: a #GdkRectangle
 * @start: a #GtkTextIter
 * @end: a #GtkTextIter
 *
 * Called when drawing a region begins. The region to be drawn is indicated
 * by @start and @end. The purpose is to allow the implementation to precompute
 * some state before the ::draw method is called for each cell.
 *
 **/
void
gtk_source_gutter_renderer_begin (GtkSourceGutterRenderer *renderer,
                                  cairo_t                 *cr,
                                  GtkWidget               *widget,
                                  const GdkRectangle      *background_area,
                                  const GdkRectangle      *cell_area,
                                  GtkTextIter             *start,
                                  GtkTextIter             *end)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));
	g_return_if_fail (cr != NULL);
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (background_area != NULL);
	g_return_if_fail (cell_area != NULL);
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	GTK_SOURCE_GUTTER_RENDERER_GET_INTERFACE (renderer)->begin (renderer,
	                                                            cr,
	                                                            widget,
	                                                            background_area,
	                                                            cell_area,
	                                                            start,
	                                                            end);
}

/**
 * gtk_source_gutter_renderer_draw:
 * @renderer: a #GtkSourceGutterRenderer
 * @cr: the cairo render context
 * @widget: the #GtkWidget
 * @background_area: a #GdkRectangle indicating the total area to be drawn
 * @cell_area: a #GdkRectangle indicating the area to draw content
 * @start: a #GtkTextIter
 * @end: a #GtkTextIter
 * @state: a #GtkSourceGutterRendererState
 *
 * Main renderering method. Implementations should implement this method to
 * draw onto the cairo context. The @background_area indicates total area of
 * the cell (without padding or margin) to be drawn. The @cell_area indicates
 * the area where content can be drawn (text, images, etc).
 *
 * The @state argument indicates the current state of the renderer and should
 * be taken into account to properly draw the different possible states
 * (cursor, prelit, selected) if appropriate.
 *
 **/
void
gtk_source_gutter_renderer_draw (GtkSourceGutterRenderer      *renderer,
                                 cairo_t                      *cr,
                                 GtkWidget                    *widget,
                                 const GdkRectangle           *background_area,
                                 const GdkRectangle           *cell_area,
                                 GtkTextIter                  *start,
                                 GtkTextIter                  *end,
                                 GtkSourceGutterRendererState  state)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));
	g_return_if_fail (cr != NULL);
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (background_area != NULL);
	g_return_if_fail (cell_area != NULL);
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	GTK_SOURCE_GUTTER_RENDERER_GET_INTERFACE (renderer)->draw (renderer,
	                                                           cr,
	                                                           widget,
	                                                           background_area,
	                                                           cell_area,
	                                                           start,
	                                                           end,
	                                                           state);
}

/**
 * gtk_source_gutter_renderer_end:
 * @renderer: a #GtkSourceGutterRenderer
 *
 * Called when drawing a region of lines has ended.
 *
 **/
void
gtk_source_gutter_renderer_end (GtkSourceGutterRenderer *renderer)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));

	GTK_SOURCE_GUTTER_RENDERER_GET_INTERFACE (renderer)->end (renderer);
}

/**
 * gtk_source_gutter_renderer_query_activatable:
 * @renderer: a #GtkSourceGutterRenderer
 * @iter: a #GtkTextIter at the start of the line to be activated
 * @area: a #GdkRectangle of the cell area to be activated
 * @event: the event that triggered the query
 *
 * Get whether the renderer is activatable at the location in @event. This is
 * called from #GtkSourceGutter to determine whether a renderer is activatable
 * using the mouse pointer.
 *
 * Returns: %TRUE if the renderer can be activated, %FALSE otherwise
 *
 **/
gboolean
gtk_source_gutter_renderer_query_activatable (GtkSourceGutterRenderer *renderer,
                                              GtkTextIter             *iter,
                                              const GdkRectangle      *area,
                                              GdkEvent                *event)
{
	gboolean ret;

	g_return_val_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (area != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	ret = FALSE;

	g_signal_emit (renderer, signals[QUERY_ACTIVATABLE], 0, iter, area, event, &ret);

	return ret;
}

/**
 * gtk_source_gutter_renderer_get_size:
 * @renderer: a #GtkSourceGutterRenderer
 * @cr: a #cairo_t
 * @width: (out caller-allocates): return value for the requested width of the renderer
 * @height: (out caller-allocates): return value for the requested height of the renderer
 *
 * Get the required size of the renderer. The @width and @height parameters
 * can be %NULL depending on whether the renderer is packed in a top/bottom or
 * left/right gutter.
 *
 **/
void
gtk_source_gutter_renderer_get_size (GtkSourceGutterRenderer *renderer,
                                     cairo_t                 *cr,
                                     GtkWidget               *widget,
                                     gint                    *width,
                                     gint                    *height)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));
	g_return_if_fail (cr != NULL);
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (width != NULL || height != NULL);

	GTK_SOURCE_GUTTER_RENDERER_GET_INTERFACE (renderer)->get_size (renderer,
	                                                               cr,
	                                                               widget,
	                                                               width,
	                                                               height);
}

/**
 * gtk_source_gutter_renderer_activate:
 * @renderer: a #GtkSourceGutterRenderer
 * @iter: a #GtkTextIter at the start of the line where the renderer is activated
 * @area: a #GdkRectangle of the cell area where the renderer is activated
 * @x: the x position (in window coordinates) where the renderer is activated
 * @y: the y position (in window coordinates) where the renderer is activated
 *
 * Emits the ::activate signal of the renderer. This is called from
 * #GtkSourceGutter and should never have to be called manually.
 *
 */
void
gtk_source_gutter_renderer_activate (GtkSourceGutterRenderer *renderer,
                                     GtkTextIter             *iter,
                                     const GdkRectangle      *area,
                                     gint                     x,
                                     gint                     y)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (area != NULL);

	g_signal_emit (renderer, signals[ACTIVATE], 0, iter, area, x, y);
}

/**
 * gtk_source_gutter_renderer_size_changed:
 * @renderer: a #GtkSourceGutterRenderer
 * 
 * Emits the ::size-changed signal of the renderer. Call this from an
 * implementation to inform about a size change of the renderer.
 *
 **/
void
gtk_source_gutter_renderer_size_changed (GtkSourceGutterRenderer *renderer)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));

	g_signal_emit (renderer, signals[SIZE_CHANGED], 0);
}

void
gtk_source_gutter_renderer_queue_draw (GtkSourceGutterRenderer *renderer)
{
	g_return_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer));

	g_signal_emit (renderer, signals[QUEUE_DRAW], 0);
}

gboolean
gtk_source_gutter_renderer_query_tooltip (GtkSourceGutterRenderer *renderer,
                                          GtkTextIter             *iter,
                                          const GdkRectangle      *area,
                                          gint                     x,
                                          gint                     y,
                                          GtkTooltip              *tooltip)
{
	gboolean ret;

	g_return_val_if_fail (GTK_IS_SOURCE_GUTTER_RENDERER (renderer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (area != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_TOOLTIP (tooltip), FALSE);

	ret = FALSE;

	g_signal_emit (renderer,
	               signals[ACTIVATE],
	               0,
	               iter,
	               area,
	               x,
	               y,
	               tooltip,
	               &ret);

	return ret;
}
