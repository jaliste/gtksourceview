#include "gtksourcegutterrenderertext.h"

#define GTK_SOURCE_GUTTER_RENDERER_TEXT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_SOURCE_GUTTER_RENDERER_TEXT, GtkSourceGutterRendererTextPrivate))

struct _GtkSourceGutterRendererTextPrivate
{
	gchar *text;

	gchar *measure_text;

	gint width;
	gint height;

	gdouble xalign;
	gdouble yalign;

	gint xpad;
	gint ypad;

	guint visible : 1;
	guint measure_is_markup : 1;
	guint is_markup : 1;
};

static void gtk_source_gutter_renderer_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GtkSourceGutterRendererText,
                         gtk_source_gutter_renderer_text,
                         G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SOURCE_GUTTER_RENDERER,
                                                gtk_source_gutter_renderer_iface_init))

enum
{
	PROP_0,
	PROP_MARKUP,
	PROP_TEXT,
	PROP_XALIGN,
	PROP_YALIGN,
	PROP_VISIBLE,
	PROP_XPAD,
	PROP_YPAD
};

static PangoLayout *
create_layout (GtkSourceGutterRendererText *renderer,
               GtkWidget                   *widget)
{
	PangoLayout *layout;
	PangoAttrList *attr_list;
	PangoAttribute *attr;
	GdkColor color;
	GtkStyle *style;
	gboolean unref_attr_list;

	layout = gtk_widget_create_pango_layout (widget, NULL);

	if (renderer->priv->is_markup)
	{
		pango_layout_set_markup (layout, renderer->priv->text, -1);
	}
	else
	{
		pango_layout_set_text (layout, renderer->priv->text, -1);
	}

	attr_list = pango_layout_get_attributes (layout);
	unref_attr_list = FALSE;

	if (attr_list == NULL)
	{
		attr_list = pango_attr_list_new ();
		unref_attr_list = TRUE;
	}

	style = gtk_widget_get_style (widget);
	color = style->fg[GTK_STATE_NORMAL];

	attr = pango_attr_foreground_new (color.red, color.green, color.blue);
	attr->start_index = 0;
	attr->end_index = G_MAXINT;

	pango_attr_list_insert (attr_list, attr);

	if (unref_attr_list)
	{
		pango_layout_set_attributes (layout, attr_list);
		pango_attr_list_unref (attr_list);
	}

	return layout;
}

static void
gutter_renderer_text_draw (GtkSourceGutterRenderer      *renderer,
                           cairo_t                      *cr,
                           GtkWidget                    *widget,
                           const GdkRectangle           *background_area,
                           const GdkRectangle           *cell_area,
                           GtkTextIter                  *start,
                           GtkTextIter                  *end,
                           GtkSourceGutterRendererState  state)
{
	PangoLayout *layout;
	GtkSourceGutterRendererText *text;
	gint width;
	gint height;

	text = GTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);

	layout = create_layout (text, widget);
	pango_layout_get_size (layout, &width, &height);

	width /= PANGO_SCALE;
	height /= PANGO_SCALE;

	gtk_paint_layout (gtk_widget_get_style (widget),
	                  cr,
	                  gtk_widget_get_state (widget),
	                  TRUE,
	                  widget,
	                  "gtksourcegutterrenderertext",
	                  cell_area->x + text->priv->xpad +
	                  (cell_area->width - width - 2 * text->priv->xpad) *
	                  text->priv->xalign,
	                  cell_area->y + text->priv->ypad +
	                  (cell_area->height - height - 2 * text->priv->ypad) *
	                  text->priv->yalign,
	                  layout);

	g_object_unref (layout);
}

static void
gutter_renderer_text_get_size (GtkSourceGutterRenderer *renderer,
                               cairo_t                 *cr,
                               GtkWidget               *widget,
                               gint                    *width,
                               gint                    *height)
{
	GtkSourceGutterRendererText *text;

	text = GTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);

	if (!width && !height)
	{
		return;
	}

	if (text->priv->width < 0 || text->priv->height < 0)
	{
		PangoLayout *layout;

		layout = gtk_widget_create_pango_layout (widget, NULL);

		if (text->priv->measure_is_markup)
		{
			pango_layout_set_markup (layout,
			                         text->priv->measure_text,
			                         -1);
		}
		else
		{
			pango_layout_set_text (layout,
			                       text->priv->measure_text,
			                       -1);
		}

		pango_layout_get_size (layout,
		                       &text->priv->width,
		                       &text->priv->height);

		text->priv->width /= PANGO_SCALE;
		text->priv->height /= PANGO_SCALE;

		g_object_unref (layout);
	}

	if (width)
	{
		*width = text->priv->width + 2 * text->priv->xpad;
	}

	if (height)
	{
		*height = text->priv->height + 2 * text->priv->ypad;
	}
}

static void
gutter_renderer_text_size_changed (GtkSourceGutterRenderer *renderer)
{
	GtkSourceGutterRendererText *text;

	text = GTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);

	g_free (text->priv->measure_text);

	text->priv->measure_text = g_strdup (text->priv->text);
	text->priv->measure_is_markup = text->priv->is_markup;
}

static void
gtk_source_gutter_renderer_iface_init (gpointer g_iface,
                                       gpointer iface_data)
{
	GtkSourceGutterRendererIface *iface = g_iface;

	iface->draw = gutter_renderer_text_draw;
	iface->get_size = gutter_renderer_text_get_size;
	iface->size_changed = gutter_renderer_text_size_changed;
}

static void
gtk_source_gutter_renderer_text_finalize (GObject *object)
{
	GtkSourceGutterRendererText *renderer = GTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	g_free (renderer->priv->text);

	G_OBJECT_CLASS (gtk_source_gutter_renderer_text_parent_class)->finalize (object);
}

static void
set_text (GtkSourceGutterRendererText *renderer,
          gchar const                 *text,
          gboolean                     is_markup)
{
	g_free (renderer->priv->text);
	g_free (renderer->priv->measure_text);

	renderer->priv->measure_text = NULL;

	renderer->priv->text = g_strdup (text);
	renderer->priv->is_markup = is_markup;
}

static void
gtk_source_gutter_renderer_text_set_property (GObject      *object,
                                              guint         prop_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
	GtkSourceGutterRendererText *renderer;

	renderer = GTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	switch (prop_id)
	{
		case PROP_MARKUP:
			set_text (renderer, g_value_get_string (value), TRUE);
		break;
		case PROP_TEXT:
			set_text (renderer, g_value_get_string (value), FALSE);
		break;
		case PROP_XALIGN:
			renderer->priv->xalign = g_value_get_double (value);
		break;
		case PROP_YALIGN:
			renderer->priv->yalign = g_value_get_double (value);
		break;
		case PROP_VISIBLE:
			renderer->priv->visible = g_value_get_boolean (value);
			gtk_source_gutter_renderer_queue_draw (GTK_SOURCE_GUTTER_RENDERER (renderer));
		break;
		case PROP_XPAD:
			renderer->priv->xpad = g_value_get_int (value);
			gtk_source_gutter_renderer_size_changed (GTK_SOURCE_GUTTER_RENDERER (renderer));
		break;
		case PROP_YPAD:
			renderer->priv->xpad = g_value_get_int (value);
			gtk_source_gutter_renderer_size_changed (GTK_SOURCE_GUTTER_RENDERER (renderer));
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_gutter_renderer_text_get_property (GObject    *object,
                                              guint       prop_id,
                                              GValue     *value,
                                              GParamSpec *pspec)
{
	GtkSourceGutterRendererText *renderer;

	renderer = GTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	switch (prop_id)
	{
		case PROP_MARKUP:
			g_value_set_string (value, renderer->priv->is_markup ? renderer->priv->text : NULL);
		break;
		case PROP_TEXT:
			g_value_set_string (value, !renderer->priv->is_markup ? renderer->priv->text : NULL);
		break;
		case PROP_XALIGN:
			g_value_set_double (value, renderer->priv->xalign);
		break;
		case PROP_YALIGN:
			g_value_set_double (value, renderer->priv->yalign);
		break;
		case PROP_VISIBLE:
			g_value_set_boolean (value, renderer->priv->visible);
		break;
		case PROP_XPAD:
			g_value_set_int (value, renderer->priv->xpad);
		break;
		case PROP_YPAD:
			g_value_set_int (value, renderer->priv->ypad);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_gutter_renderer_text_class_init (GtkSourceGutterRendererTextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gtk_source_gutter_renderer_text_finalize;

	object_class->get_property = gtk_source_gutter_renderer_text_get_property;
	object_class->set_property = gtk_source_gutter_renderer_text_set_property;

	g_type_class_add_private (object_class, sizeof(GtkSourceGutterRendererTextPrivate));

	g_object_class_install_property (object_class,
	                                 PROP_MARKUP,
	                                 g_param_spec_string ("markup",
	                                                      "Markup",
	                                                      "Markup",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_TEXT,
	                                 g_param_spec_string ("text",
	                                                      "Text",
	                                                      "Text",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


	g_object_class_install_property (object_class,
	                                 PROP_XALIGN,
	                                 g_param_spec_double ("xalign",
	                                                      "Xalign",
	                                                      "Xalign",
	                                                      0,
	                                                      1,
	                                                      0,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


	g_object_class_install_property (object_class,
	                                 PROP_YALIGN,
	                                 g_param_spec_double ("yalign",
	                                                      "Yalign",
	                                                      "Yalign",
	                                                      0,
	                                                      1,
	                                                      0,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


	g_object_class_override_property (object_class,
	                                  PROP_VISIBLE,
	                                  "visible");


	g_object_class_install_property (object_class,
	                                 PROP_XPAD,
	                                 g_param_spec_int ("xpad",
	                                                   "Xpad",
	                                                   "Xpad",
	                                                   G_MININT,
	                                                   G_MAXINT,
	                                                   0,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


	g_object_class_install_property (object_class,
	                                 PROP_YPAD,
	                                 g_param_spec_int ("ypad",
	                                                   "Ypad",
	                                                   "Ypad",
	                                                   G_MININT,
	                                                   G_MAXINT,
	                                                   0,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
gtk_source_gutter_renderer_text_init (GtkSourceGutterRendererText *self)
{
	self->priv = GTK_SOURCE_GUTTER_RENDERER_TEXT_GET_PRIVATE (self);

	self->priv->width = -1;
	self->priv->height = -1;
	self->priv->is_markup = TRUE;
	self->priv->xalign = 0;
	self->priv->yalign = 0;
}

/**
 * gtk_source_gutter_renderer_text_new:
 *
 * Create a new #GtkSourceGutterRendererText.
 *
 * Returns: (transfer full): A #GtkSourceGutterRenderer
 *
 **/
GtkSourceGutterRenderer *
gtk_source_gutter_renderer_text_new ()
{
	return g_object_new (GTK_TYPE_SOURCE_GUTTER_RENDERER_TEXT, NULL);
}
