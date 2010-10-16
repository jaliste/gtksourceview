#ifndef __GTK_SOURCE_PIXBUF_HELPER_H__
#define __GTK_SOURCE_PIXBUF_HELPER_H__

#include <gtk/gtk.h>

typedef struct _GtkSourcePixbufHelper GtkSourcePixbufHelper;

GtkSourcePixbufHelper *gtk_source_pixbuf_helper_new (void);
void gtk_source_pixbuf_helper_free (GtkSourcePixbufHelper *helper);

void gtk_source_pixbuf_helper_set_pixbuf (GtkSourcePixbufHelper *helper,
                                          const GdkPixbuf       *pixbuf);

GdkPixbuf *gtk_source_pixbuf_helper_get_pixbuf (GtkSourcePixbufHelper *helper);

void gtk_source_pixbuf_helper_set_stock_id (GtkSourcePixbufHelper *helper,
                                            const gchar           *stock_id);

const gchar *gtk_source_pixbuf_helper_get_stock_id (GtkSourcePixbufHelper *helper);

void gtk_source_pixbuf_helper_set_stock_detail (GtkSourcePixbufHelper *helper,
                                                const gchar           *stock_id);

const gchar *gtk_source_pixbuf_helper_get_stock_detail (GtkSourcePixbufHelper *helper);

void gtk_source_pixbuf_helper_set_icon_name (GtkSourcePixbufHelper *helper,
                                             const gchar           *icon_name);

const gchar *gtk_source_pixbuf_helper_get_icon_name (GtkSourcePixbufHelper *helper);

void gtk_source_pixbuf_helper_set_gicon (GtkSourcePixbufHelper *helper,
                                         GIcon                 *gicon);

GIcon *gtk_source_pixbuf_helper_get_gicon (GtkSourcePixbufHelper *helper);

GdkPixbuf *gtk_source_pixbuf_helper_render (GtkSourcePixbufHelper *helper,
                                            GtkWidget             *widget,
                                            gint                   size);

#endif /* __GTK_SOURCE_PIXBUF_HELPER_H__ */

