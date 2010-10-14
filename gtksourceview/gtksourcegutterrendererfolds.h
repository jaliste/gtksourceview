#ifndef __GTK_SOURCE_GUTTER_RENDERER_FOLDS_H__
#define __GTK_SOURCE_GUTTER_RENDERER_FOLDS_H__

#include <gtk/gtk.h>
#include <glib-object.h>

#define GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS              (gtk_source_gutter_renderer_folds_get_type())
#define GTK_SOURCE_GUTTER_RENDERER_FOLDS(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, GtkSourceGutterRendererFolds))
#define GTK_SOURCE_GUTTER_RENDERER_FOLDS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, GtkSourceGutterRendererFoldsClass))
#define GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS))
#define GTK_IS_SOURCE_GUTTER_RENDERER_FOLDS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS))
#define GTK_SOURCE_GUTTER_RENDERER_FOLDS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GTK_TYPE_SOURCE_GUTTER_RENDERER_FOLDS, GtkSourceGutterRendererFoldsClass))

typedef struct _GtkSourceGutterRendererFolds GtkSourceGutterRendererFolds;
typedef struct _GtkSourceGutterRendererFoldsClass GtkSourceGutterRendererFoldsClass;

typedef struct _GtkSourceGutterRendererFoldsPrivate GtkSourceGutterRendererFoldsPrivate;

typedef enum
{
	GTK_SOURCE_FOLD_MARK_NONE,
	GTK_SOURCE_FOLD_MARK_START,
	GTK_SOURCE_FOLD_MARK_STOP,
	GTK_SOURCE_FOLD_MARK_INTERIOR,
	GTK_SOURCE_FOLD_MARK_START_FOLDED
} GtkSourceFoldMarkType;


struct _GtkSourceGutterRendererFolds
{
	GtkCellRenderer   parent;

	/*< private >*/
	GtkSourceGutterRendererFoldsPrivate *priv;
};


struct _GtkSourceGutterRendererFoldsClass
{
	GtkCellRendererClass  parent_class;

	/* Padding for future expansion */
	void (*_gtk_source_reserved1) 	(void);
	void (*_gtk_source_reserved2) 	(void);
	void (*_gtk_source_reserved3) 	(void);
};


GType			 gtk_source_gutter_renderer_folds_get_type (void);

GtkCellRenderer		*gtk_source_gutter_renderer_folds_new (void);

void			 gtk_source_gutter_renderer_folds_set_fold_mark (GtkSourceGutterRendererFolds *cell,
								      GtkSourceFoldMarkType fold_mark);
GtkSourceFoldMarkType
			 gtk_source_gutter_renderer_folds_get_fold_mark (GtkSourceGutterRendererFolds *cell);

void			 gtk_source_gutter_renderer_folds_set_depth (GtkSourceGutterRendererFolds *cell,
								  guint depth);
guint			 gtk_source_gutter_renderer_folds_get_depth (GtkSourceGutterRendererFolds *cell);

#endif /* __GTK_SOURCE_GUTTER_RENDERER_FOLDS_H__ */
