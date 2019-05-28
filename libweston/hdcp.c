#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libweston/libweston.h>
#include "weston-hdcp-server-protocol.h"

struct weston_hdcp_data {
	int connector_id;
	int value;
};


static void enable_disable_hdcp(int fd, uint32_t connector_id, 
				int request_value) {
	int i;
	drmModeResPtr pDrmModeResPtr = NULL;
	drmModePropertyPtr prop;
	drmModeObjectPropertiesPtr props;
	drmModeConnectorPtr c = NULL;

	pDrmModeResPtr = drmModeGetResources(fd);
	if(pDrmModeResPtr == NULL) {
		weston_log("Failed to get drmMode Resources\n");
		goto done;
	}

done:
	return;


}

static void hdcp_set_property(struct wl_client *client,
			      struct wl_resource *resource,
			      int32_t hdcp_drm_fd,
		              int32_t hdcp_connector_id,
			      int32_t hdcp_value) {
	
	weston_log("hdcp_connector_id = %d\n", hdcp_drm_fd);
	weston_log("hdcp_connector_id = %d\n", hdcp_connector_id);
	weston_log("hdcp_value = %d\n", hdcp_value);
	return;
}

struct weston_hdcp_interface weston_hdcp_iface = {
	hdcp_set_property
};

static void bind_weston_hdcp(struct wl_client *client,
			     void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource;

	resource = wl_resource_create(client, &weston_hdcp_interface,
				      version, id);

	if(resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &weston_hdcp_iface,
				       data, NULL);
}

WL_EXPORT void 
weston_hdcp_create(struct weston_compositor *ec) {
	struct weston_hdcp_data *data;
	
	data = zalloc(sizeof *data);
	if(data == NULL)
		return;

	if(!wl_global_create(ec->wl_display, &weston_hdcp_interface, 1,
			     data, bind_weston_hdcp))
		return;
}
