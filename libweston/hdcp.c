#include <inttypes.h>
#include <string.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libweston/libweston.h>
#include "weston-hdcp-server-protocol.h"

#define CONTENT_PROTECTION "Content Protection"

struct weston_hdcp_data {
	int32_t drm_fd;
	int32_t connector_id;
	int32_t value;
};

static void getContentProtectionProp(int drmFd, drmModeResPtr res, int32_t *current_val) {

        uint32_t i;
        drmModePropertyPtr prop;
        drmModeObjectPropertiesPtr props;

        for (i = 0; i < (uint32_t)res->count_connectors; i++) {
                drmModeConnectorPtr pDrmModeConnectorPtr = drmModeGetConnector(drmFd, res->connectors[i]);
                if(!pDrmModeConnectorPtr)
                        continue;
                if(pDrmModeConnectorPtr->connection==DRM_MODE_CONNECTED) {

                        props = drmModeObjectGetProperties(drmFd,
                                                pDrmModeConnectorPtr->connector_id,
                                                DRM_MODE_OBJECT_CONNECTOR);

                        for (i = 0; i < props->count_props; i++) {
				prop = drmModeGetProperty(drmFd, props->props[i]);
				if (strcmp(prop->name, CONTENT_PROTECTION) == 0)
				{
					weston_log("%s\n", prop->name);
					weston_log("%d\n", prop->prop_id);
					if (!(drm_property_type_is(prop, DRM_MODE_PROP_BLOB)) ||
					     (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))) {
						weston_log("%" PRId64 "\n", props->prop_values[i]);
						current_val = (int32_t *)&(props->prop_values[i]);
						weston_log("%" PRId64 "\n", (uint64_t)current_val);
					}
				}
			}
		}
	}
        drmModeFreeProperty(prop);
}

static int open_drm() {

	int32_t fd;

	fd = drmOpen("i915", NULL);

	return fd;
}

static void enable_disable_hdcp(int32_t connector_id,
				int32_t req_val) {
	int32_t i, curr_val;
	uint32_t j, sts;
	drmModeResPtr p_drmModeResPtr;
	int fd;

	fd = open_drm();
	if(fd < 0) {
		weston_log("Failed to Open drm device\n");
		goto done;
	}

	p_drmModeResPtr = drmModeGetResources(fd);
	if(p_drmModeResPtr == NULL) {
		weston_log("Failed to get drmMode Resources\n");
		goto done;
	}

	for(i = 0; i < p_drmModeResPtr->count_connectors; i++) {
					drmModeConnectorPtr c = drmModeGetConnector(fd,
					p_drmModeResPtr->connectors[i]);
		if(!c)
			continue;

		if(c->connection == DRM_MODE_CONNECTED) {
			drmModeObjectPropertiesPtr props;
			props = drmModeObjectGetProperties(fd,
						c->connector_id,
						DRM_MODE_OBJECT_CONNECTOR);

			for (j = 0; j < props->count_props; j++) {
				drmModePropertyPtr prop;
				prop = drmModeGetProperty(fd, props->props[i]);
				weston_log("drmModeConnectorSetProperty returns: %d\n",
						sts);
				getContentProtectionProp(fd, p_drmModeResPtr, &curr_val);
				if(req_val != curr_val)
						sts = drmModeConnectorSetProperty(fd,
							connector_id,prop->prop_id,req_val);
			}
		}
	}
done:
	return;
}

static void hdcp_set_property(struct wl_client *client,
			      struct wl_resource *resource,
		              int32_t hdcp_connector_id,
			      int32_t hdcp_value) {
	enable_disable_hdcp(hdcp_connector_id, hdcp_value);
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
