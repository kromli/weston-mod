#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wayland-client.h>
#include "weston-hdcp-client-protocol.h"
#include <wayland-server-core.h>

struct hdcp_data {
	int32_t drm_fd;
	int32_t connector_id;
	int32_t value;
	struct wl_list output_list;
	struct weston_hdcp *w_hdcp;
};

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	int compositor_version;
};

static void weston_hdcp_done(void *data, struct weston_hdcp *weston_hdcp)  {
	printf("%s:%d\n", __FUNCTION__, __LINE__);	
}

static const struct weston_hdcp_listener w_hdcp_listener = {
        weston_hdcp_done
};

static void handle_global(void *data, struct wl_registry *registry,
			  uint32_t name, const char *interface, uint32_t version) {
	struct hdcp_data *k_data = data;

	if(strcmp(interface, "weston_hdcp") == 0) {
		k_data->w_hdcp = wl_registry_bind(registry, name, &weston_hdcp_interface, 1);
		//printf("Bind OK\n");
	} else {
		return;	
	}
}

static void
handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
	/* XXX: unimplemented */
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove
};

int main(int argc, char **argv) {
	struct wl_display *display;
	struct wl_registry *registry;
	struct hdcp_data data = {};

	display = wl_display_connect(NULL);
	if (NULL == display) {
		fprintf(stderr, "failed to create display: %s\n",
			strerror(errno));
		return -1;
	}

	wl_list_init(&data.output_list);
	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &data);
	wl_display_dispatch(display);
	wl_display_roundtrip(display);
	
	if (NULL == data.w_hdcp) {	
		printf("%d:Error\n", __LINE__); 
		return -1;	
	}
	weston_hdcp_add_listener(data.w_hdcp, &w_hdcp_listener, &data);

	printf("Please enter integer to be passed to backend: ");
	scanf("%d", &data.drm_fd);
	
	printf("Please enter integer to be passed to backend: ");
	scanf("%d", &data.connector_id);
	
	printf("Please enter integer to be passed to backend: ");
	scanf("%d", &data.value);

//	wl_display_dispatch(display);
	weston_hdcp_set_property(data.w_hdcp, data.drm_fd, 
				 data.connector_id, data.value);
	wl_display_roundtrip(display);
	wl_registry_destroy(registry);
	wl_display_flush(display);
	wl_display_disconnect(display);

	printf("Hey Hey Hey\n");
	return 0;
}
