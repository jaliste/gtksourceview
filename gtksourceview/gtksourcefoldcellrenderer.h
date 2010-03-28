#ifndef __GTK_SOURCE_FOLD_CELL_RENDERER_H__
#define __GTK_SOURCE_FOLD_CELL_RENDERER_H__

#include <gtk/gtk.h>
#include <glib-object.h>

#define GTK_TYPE_SOURCE_FOLD_CELL_RENDERER              (gtk_source_fold_cell_renderer_get_type())
#define GTK_SOURCE_FOLD_CELL_RENDERER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  GTK_TYPE_SOURCE_FOLD_CELL_RENDERER, GtkSourceFoldCellRenderer))
#define GTK_SOURCE_FOLD_CELL_RENDERER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GTK_TYPE_SOURCE_FOLD_CELL_RENDERER, GtkSourceFoldCellRendererClass))
#define GTK_IS_SOURCE_FOLD_CELL_RENDERER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_FOLD_CELL_RENDERER))
#define GTK_IS_SOURCE_FOLD_CELL_RENDERER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTK_TYPE_SOURCE_FOLD_CELL_RENDERER))
#define GTK_SOURCE_FOLD_CELL_RENDERER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GTK_TYPE_SOURCE_FOLD_CELL_RENDERER, GtkSourceFoldCellRendererClass))

typedef struct _GtkSourceFoldCellRenderer GtkSourceFoldCellRenderer;
typedef struct _GtkSourceFoldCellRendererClass GtkSourceFoldCellRendererClass;

typedef struct _GtkSourceFoldCellRendererPrivate GtkSourceFoldCellRendererPrivate;

typedef enum
{
	GTK_SOURCE_FOLD_MARK_NONE,
	GTK_SOURCE_FOLD_MARK_START,
	GTK_SOURCE_FOLD_MARK_STOP,
	GTK_SOURCE_FOLD_MARK_INTERIOR,
	GTK_SOURCE_FOLD_MARK_START_FOLDED
} GtkSourceFoldMarkType;


struct _GtkSourceFoldCellRenderer
{
	GtkCellRenderer   parent;

	/*< private >*/
	GtkSourceFoldCellRendererPrivate *priv;
};


struct _GtkSourceFoldCellRendererClass
{
	GtkCellRendererClass  parent_class;

	/* Signals */
	void (* toggled) (GtkSourceFoldCellRenderer	*cell_renderer,
			  gchar				*path);

	/* Padding for future expansion */
	void (*_gtk_source_reserved1) 	(void);
	void (*_gtk_source_reserved2) 	(void);
	void (*_gtk_source_reserved3) 	(void);
};


GType			 gtk_source_fold_cell_renderer_get_type		(void);

GtkCellRenderer		*gtk_source_fold_cell_renderer_new		(void);

void			 gtk_source_fold_cell_renderer_set_fold_mark	(GtkSourceFoldCellRenderer *cell,
									 GtkSourceFoldMarkType fold_mark);
GtkSourceFoldMarkType
			 gtk_source_fold_cell_renderer_get_fold_mark	(GtkSourceFoldCellRenderer *cell);

void			 gtk_source_fold_cell_renderer_set_depth	(GtkSourceFoldCellRenderer *cell,
									 guint depth);
guint			 gtk_source_fold_cell_renderer_get_depth	(GtkSourceFoldCellRenderer *cell);

void			 gtk_source_fold_cell_renderer_set_percent_fill	(GtkSourceFoldCellRenderer *cell,
									 gfloat percent_fill);
gfloat			 gtk_source_fold_cell_renderer_get_percent_fill	(GtkSourceFoldCellRenderer *cell);

#endif /* __GTK_SOURCE_FOLD_CELL_RENDERER_H__ */
