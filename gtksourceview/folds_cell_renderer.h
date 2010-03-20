#ifndef _folds_cell_renderer_included_
#define _folds_cell_renderer_included_

#include <gtk/gtk.h>
#include <glib-object.h>

#define FOLDS_TYPE_CELL_RENDERER              (folds_cell_renderer_get_type())
#define FOLDS_CELL_RENDERER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  FOLDS_TYPE_CELL_RENDERER, FoldsCellRenderer))
#define FOLDS_CELL_RENDERER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  FOLDS_TYPE_CELL_RENDERER, FoldsCellRendererClass))
#define FOLDS_IS_CELL__(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FOLDS_TYPE_CELL_RENDERER))
#define FOLDS_IS_CELL___CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  FOLDS_TYPE_CELL_RENDERER))
#define FOLDS_CELL_RENDERER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  FOLDS_TYPE_CELL_RENDERER, FoldsCellRendererClass))

typedef struct _FoldsCellRenderer FoldsCellRenderer;
typedef struct _FoldsCellRendererClass FoldsCellRendererClass;

enum
{
	FOLD_MARK_NULL,
	FOLD_MARK_START,
	FOLD_MARK_STOP,
	FOLD_MARK_INTERIOR,
	FOLD_MARK_START_FOLDED
};

struct _FoldsCellRenderer
{
  GtkCellRenderer   parent;

  gint	      fold_mark;	/* which fold mark to draw */
  gint	      depth;		/* is it folded? */
};


struct _FoldsCellRendererClass
{
  GtkCellRendererClass  parent_class;
};


GType                folds_cell_renderer_get_type (void);

GtkCellRenderer     *folds_cell_renderer_new (void);


#endif /* _folds_cell_renderer_included_ */
