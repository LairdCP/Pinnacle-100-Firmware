/**
 * @file lcz_lwm2m_sensor.c
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(lwm2m_sensor, CONFIG_LCZ_LWM2M_SENSOR_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <sys/atomic.h>
#include <net/lwm2m.h>

#include "lwm2m_resource_ids.h"
#include "ipso_filling_sensor.h"

#include "lcz_bt_scan.h"
#include "lcz_sensor_event.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_adv_match.h"
#include "lcz_lwm2m_client.h"
#include "lcz_lwm2m_gateway.h"
#include "errno_str.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define LWM2M_INSTANCES_PER_SENSOR_MAX 4

/* clang-format off */
#define LWM2M_BT610_TEMPERATURE_UNITS "C"
#define LWM2M_BT610_TEMPERATURE_MIN   -40.0
#define LWM2M_BT610_TEMPERATURE_MAX   125.0

#define LWM2M_BT610_CURRENT_UNITS "A"
#define LWM2M_BT610_CURRENT_MIN   0.0
#define LWM2M_BT610_CURRENT_MAX   500.0

#define LWM2M_BT610_PRESSURE_UNITS "PSI"
#define LWM2M_BT610_PRESSURE_MIN   0
#define LWM2M_BT610_PRESSURE_MAX   1000.0
/* clang-format on */

struct lwm2m_sensor_table {
	bt_addr_t addr;
	uint8_t last_record_type;
	uint16_t last_event_id;
	uint16_t base; /* instance */
	char name[SENSOR_NAME_MAX_SIZE];
};

#define CONFIGURATOR(_type, _instance, _units, _min, _max, _skip)              \
	cfg.type = (_type);                                                    \
	cfg.instance = (_instance);                                            \
	cfg.units = (_units);                                                  \
	cfg.min = (_min);                                                      \
	cfg.max = (_max);                                                      \
	cfg.skip_secondary = (_skip)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static struct {
	bool initialized;
	int scan_user_id;
	uint32_t ads;
	uint32_t sensor_ads;
	uint32_t accepted_ads;
	uint8_t sensor_count;
	struct lwm2m_sensor_table table[CONFIG_LCZ_LWM2M_SENSOR_MAX];
	bool not_enough_instances;
	bool gen_instance_error;
} ls;

static struct lwm2m_sensor_table *const lst = ls.table;

/* Each BT610 can have multiple sensors */
static ATOMIC_DEFINE(ls_sensor_created, (CONFIG_LCZ_LWM2M_SENSOR_MAX *
					 LWM2M_INSTANCES_PER_SENSOR_MAX));

static ATOMIC_DEFINE(ls_gateway_created, CONFIG_LCZ_LWM2M_SENSOR_MAX);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void ad_handler(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
		       struct net_buf_simple *ad);

static bool ad_discard(LczSensorAdEvent_t *p);
static void ad_filter(LczSensorAdEvent_t *p, int8_t rssi);
static void ad_process(LczSensorAdEvent_t *p, uint8_t idx, int8_t rssi);

static int get_index(const bt_addr_t *addr, bool allow_gen);
static int generate_new_base(const bt_addr_t *addr, size_t idx);
static bool valid_base(uint16_t instance);

static int create_sensor_obj(struct lwm2m_sensor_obj_cfg *cfg, uint8_t idx,
			     uint8_t offset);

static int create_gateway_obj(uint8_t idx, int8_t rssi);

static void obj_not_found_handler(int status, uint8_t idx, uint8_t offset);

static void name_handler(const bt_addr_le_t *addr, struct net_buf_simple *ad);

static struct float32_value make_float_value(float v);
static int lwm2m_set_sensor_data(uint16_t type, uint16_t instance, float value);

static void configure_filling_sensor(uint16_t instance);

static int register_post_write_callback(uint16_t type, uint16_t instance,
					uint16_t resource,
					lwm2m_engine_set_data_cb_t cb);
static int fill_sensor_write_cb(uint16_t obj_inst_id, uint16_t res_id,
				uint16_t res_inst_id, uint8_t *data,
				uint16_t data_len, bool last_block,
				size_t total_size);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void lcz_lwm2m_sensor_init(void)
{
	if (!lcz_bt_scan_register(&ls.scan_user_id, ad_handler)) {
		LOG_ERR("LWM2M sensor module failed to register with scan module");
	}

	lcz_bt_scan_start(ls.scan_user_id);
}

/******************************************************************************/
/* Occurs in BT RX Thread context                                             */
/******************************************************************************/
static void ad_handler(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
		       struct net_buf_simple *ad)
{
	AdHandle_t handle = AdFind_Type(
		ad->data, ad->len, BT_DATA_MANUFACTURER_DATA, BT_DATA_INVALID);

	ls.ads += 1;

	if (lcz_sensor_adv_match_1m(&handle)) {
		ls.sensor_ads += 1;
		ad_filter((LczSensorAdEvent_t *)handle.pPayload, rssi);
	}

	if (lcz_sensor_adv_match_coded(&handle)) {
		ls.sensor_ads += 1;
		/* The coded phy contains the TLVs of the 1M ad and scan response */
		LczSensorAdCoded_t *coded =
			(LczSensorAdCoded_t *)handle.pPayload;
		ad_filter(&coded->ad, rssi);
	}

	name_handler(addr, ad);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void name_handler(const bt_addr_le_t *addr, struct net_buf_simple *ad)
{
	AdHandle_t handle = AdFind_Name(ad->data, ad->len);
	char *name = "?";
	int i;
	int r;

	if (handle.pPayload == NULL) {
		return;
	}

	/* Only process names for devices already in the table */
	i = get_index(&addr->a, false);
	if (i < 0) {
		return;
	}

	/* Don't start processing name until after gateway object has been
	 * created.  Then the name will be updated only if it changes.
	 */
	if (!atomic_test_bit(ls_gateway_created, i)) {
		return;
	}

	name = ls.table[i].name;
	if (strncmp(name, handle.pPayload, MAX(strlen(name), handle.size)) !=
	    0) {
		memset(name, 0, SENSOR_NAME_MAX_SIZE);
		strncpy(name, handle.pPayload,
			MIN(SENSOR_NAME_MAX_STR_LEN, handle.size));
		r = lcz_lwm2m_gateway_id_set(lst[i].base, name);
		LOG_INF("Updating name in table: %s idx: %d inst: %d lwm2m status: %d",
			log_strdup(name), i, lst[i].base, r);
	}
}

static void ad_filter(LczSensorAdEvent_t *p, int8_t rssi)
{
	if (p == NULL) {
		return;
	}

	if (ad_discard(p)) {
		return;
	}

	int i = get_index(&p->addr, true);
	if (i < 0) {
		return;
	}

	/* Filter out duplicate events.
	 * If both devices have just powered-up, don't filter event 0.
	 */
	if (p->id != 0 && p->id == lst[i].last_event_id &&
	    p->recordType == lst[i].last_record_type) {
		return;
	}

	LOG_INF("%s idx: %d base: %u RSSI: %d",
		lcz_sensor_event_get_string(p->recordType), i, lst[i].base,
		rssi);

	lst[i].last_event_id = p->id;
	lst[i].last_record_type = p->recordType;
	ls.accepted_ads += 1;

	ad_process(p, i, rssi);
}

/* Don't create a table entry for a sensor reporting
 * events that aren't going to be processed.
 */
static bool ad_discard(LczSensorAdEvent_t *p)
{
	switch (p->recordType) {
#ifdef CONFIG_LCZ_LWM2M_SENSOR_ALLOW_BT510
	case SENSOR_EVENT_TEMPERATURE:
		return false;
#endif
	case SENSOR_EVENT_TEMPERATURE_1:
	case SENSOR_EVENT_TEMPERATURE_2:
	case SENSOR_EVENT_TEMPERATURE_3:
	case SENSOR_EVENT_TEMPERATURE_4:
	case SENSOR_EVENT_CURRENT_1:
	case SENSOR_EVENT_CURRENT_2:
	case SENSOR_EVENT_CURRENT_3:
	case SENSOR_EVENT_CURRENT_4:
	case SENSOR_EVENT_PRESSURE_1:
	case SENSOR_EVENT_PRESSURE_2:
	case SENSOR_EVENT_ULTRASONIC_1:
		return false;
	default:
		return true;
	}
}

/* The event type determines the LwM2M object type.
 * The address in advertisement is used to generate instance.
 * Objects are created as advertisements are processed.
 */
static void ad_process(LczSensorAdEvent_t *p, uint8_t idx, int8_t rssi)
{
	int r = 0;
	uint8_t offset = 0;
	float f = p->data.f;
	struct lwm2m_sensor_obj_cfg cfg;

	switch (p->recordType) {
	case SENSOR_EVENT_TEMPERATURE:
		f = ((float)((int16_t)p->data.u16)) / 100.0;
		CONFIGURATOR(IPSO_OBJECT_TEMP_SENSOR_ID, lst[idx].base,
			     LWM2M_TEMPERATURE_UNITS, LWM2M_TEMPERATURE_MIN,
			     LWM2M_TEMPERATURE_MAX, false);
		break;
	case SENSOR_EVENT_TEMPERATURE_1:
	case SENSOR_EVENT_TEMPERATURE_2:
	case SENSOR_EVENT_TEMPERATURE_3:
	case SENSOR_EVENT_TEMPERATURE_4:
		offset = (p->recordType - SENSOR_EVENT_TEMPERATURE_1);
		CONFIGURATOR(IPSO_OBJECT_TEMP_SENSOR_ID, lst[idx].base + offset,
			     LWM2M_BT610_TEMPERATURE_UNITS,
			     LWM2M_BT610_TEMPERATURE_MIN,
			     LWM2M_BT610_TEMPERATURE_MAX, false);
		break;
	case SENSOR_EVENT_CURRENT_1:
	case SENSOR_EVENT_CURRENT_2:
	case SENSOR_EVENT_CURRENT_3:
	case SENSOR_EVENT_CURRENT_4:
		offset = (p->recordType - SENSOR_EVENT_CURRENT_1);
		CONFIGURATOR(IPSO_OBJECT_CURRENT_SENSOR_ID,
			     lst[idx].base + offset, LWM2M_BT610_CURRENT_UNITS,
			     LWM2M_BT610_CURRENT_MIN, LWM2M_BT610_CURRENT_MAX,
			     false);
		break;
	case SENSOR_EVENT_PRESSURE_1:
	case SENSOR_EVENT_PRESSURE_2:
		offset = (p->recordType - SENSOR_EVENT_PRESSURE_1);
		CONFIGURATOR(IPSO_OBJECT_PRESSURE_ID, (lst[idx].base + offset),
			     LWM2M_BT610_PRESSURE_UNITS,
			     LWM2M_BT610_PRESSURE_MIN, LWM2M_BT610_PRESSURE_MAX,
			     false);
		break;
	case SENSOR_EVENT_ULTRASONIC_1:
		/* Convert from mm (reported) to cm (filling sensor) */
		f /= 10.0;
		/* Units/min/max not used because filling sensor object has
		 * different resources.
		 */
		CONFIGURATOR(IPSO_OBJECT_FILLING_LEVEL_SENSOR_ID, lst[idx].base,
			     "", 0, 0, true);
		break;
	default:
		/* Only some of the events are processed */
		LOG_WRN("Event type not supported");
		r = -ENOTSUP;
		break;
	}

	/* Update the sensor data */
	if (r == 0) {
		r = create_sensor_obj(&cfg, idx, offset);

		if (r == 0) {
			r = lwm2m_set_sensor_data(cfg.type, cfg.instance, f);
			obj_not_found_handler(r, idx, offset);
		}

		if (r < 0) {
			LOG_ERR("Unable to set LwM2M sensor data: %d", r);
		}
	}

	/* Update the RSSI in the gateway object */
	if (r == 0) {
		r = create_gateway_obj(idx, rssi);

		if (r == 0) {
			r = lcz_lwm2m_gateway_rssi_set(lst[idx].base, rssi);
		}

		if (r < 0) {
			LOG_ERR("Unable to set LwM2M rssi: %d", r);
		}
	}
}

/* Don't create sensor object instances until data is received.
 *
 * The number of instances of each type of sensor object
 * is limited at compile time.
 */
static int create_sensor_obj(struct lwm2m_sensor_obj_cfg *cfg, uint8_t idx,
			     uint8_t offset)
{
	uint32_t index_with_offset =
		(idx * LWM2M_INSTANCES_PER_SENSOR_MAX) + offset;
	int r = 0;

	if (atomic_test_bit(ls_sensor_created, index_with_offset)) {
		return r;
	}

	r = lwm2m_create_sensor_obj(cfg);
	if (r == 0) {
		atomic_set_bit(ls_sensor_created, index_with_offset);
		if (cfg->type == IPSO_OBJECT_FILLING_LEVEL_SENSOR_ID) {
			configure_filling_sensor(cfg->instance);
		}
	} else if (r == -EEXIST) {
		r = 0;
		atomic_set_bit(ls_sensor_created, index_with_offset);
		LOG_WRN("object already exists");
	} else if (r == -ENOMEM) {
		ls.not_enough_instances = true;
	}

	return r;
}

static void obj_not_found_handler(int status, uint8_t idx, uint8_t offset)
{
	uint32_t index_with_offset =
		(idx * LWM2M_INSTANCES_PER_SENSOR_MAX) + offset;

	if (status == -ENOENT) {
		/* Objects can be deleted from cloud */
		LOG_WRN("object not found after creation");
		atomic_clear_bit(ls_sensor_created, index_with_offset);
	}
}

static int get_index(const bt_addr_t *addr, bool allow_gen)
{
	size_t i;
	for (i = 0; i < ls.sensor_count; i++) {
		if (bt_addr_cmp(addr, &lst[i].addr) == 0) {
			return i;
		}
	}

	if (allow_gen && i < CONFIG_LCZ_LWM2M_SENSOR_MAX) {
		return generate_new_base(addr, i);
	} else {
		if (allow_gen) {
			LOG_ERR("LwM2M sensor instance table full");
		}
		return -1;
	}
}

static int generate_new_base(const bt_addr_t *addr, size_t idx)
{
	uint16_t instance = 0;

	/* Instance is limited to 16 bits by LwM2M specification
	 * 0-3 are reserved
	 * Allow up to 4 instances of each sensor per BTx.
	 * The address is used because instance must remain constant.
	 */
	memcpy(&instance, &addr->val, 2);
	instance <<= 2;

	if (valid_base(instance)) {
		bt_addr_copy(&lst[idx].addr, addr);
		lst[idx].base = instance;
		ls.sensor_count += 1;
		return idx;
	} else {
		LOG_ERR("Unable to generate valid instance");
		return -1;
	}
}

/* This cannot prevent a duplicate from assuming the role of another sensor
 * if a new sensor is added when the gateway is disabled.
 */
static bool valid_base(uint16_t instance)
{
	size_t i;

	if (instance < LWM2M_INSTANCE_SENSOR_START) {
		ls.gen_instance_error = true;
		return false;
	}

	/* Don't allow duplicates */
	for (i = 0; i < ls.sensor_count; i++) {
		if (instance == lst[i].base) {
			ls.gen_instance_error = true;
			return false;
		}
	}

	return true;
}

/* Create object if it doesn't exist */
static int create_gateway_obj(uint8_t idx, int8_t rssi)
{
	int r = 0;
	char prefix[CONFIG_LWM2M_GATEWAY_PREFIX_MAX_STR_SIZE];
	struct lwm2m_gateway_obj_cfg cfg = {
		.instance = lst[idx].base,
		.id = lst[idx].name,
		.prefix = prefix,
		.iot_device_objects = NULL,
		.rssi = rssi,
	};

	if (atomic_test_bit(ls_gateway_created, idx)) {
		return r;
	}

#ifdef CONFIG_LCZ_LWM2M_SENSOR_ADD_PREFIX_TO_BDA
	snprintk(prefix, sizeof(prefix), "n-%u-%02X%02X%02X%02X%02X%02X",
		 lst[idx].addr.val[5], lst[idx].addr.val[4],
		 lst[idx].addr.val[3], lst[idx].addr.val[2],
		 lst[idx].addr.val[1], lst[idx].addr.val[0], lst[idx].base);
#else
	snprintk(prefix, sizeof(prefix), "n-%u", lst[idx].base);
#endif

	r = lcz_lwm2m_gateway_create(&cfg);
	if (r == 0) {
		atomic_set_bit(ls_gateway_created, idx);
	} else if (r == -EEXIST) {
		r = 0;
		atomic_set_bit(ls_gateway_created, idx);
		LOG_WRN("gateway object already exists");
	} else if (r == -ENOMEM) {
		ls.not_enough_instances = true;
	}

	return r;
}

static struct float32_value make_float_value(float v)
{
	struct float32_value f;

	f.val1 = (int32_t)v;
	f.val2 = (int32_t)(LWM2M_FLOAT32_DEC_MAX * (v - f.val1));

	return f;
}

static int lwm2m_set_sensor_data(uint16_t type, uint16_t instance, float value)
{
	char path[CONFIG_LWM2M_PATH_MAX_SIZE];
	struct float32_value float_value = make_float_value(value);
	uint16_t resource = (type == IPSO_OBJECT_FILLING_LEVEL_SENSOR_ID) ?
					  ACTUAL_FILL_LEVEL_FILLING_SENSOR_RID :
					  SENSOR_VALUE_RID;

	snprintk(path, sizeof(path), "%u/%u/%u", type, instance, resource);

	return lwm2m_engine_set_float32(path, &float_value);
}

/*
 * Save and load filling sensor config to the file system
 */
static void configure_filling_sensor(uint16_t instance)
{
	const uint16_t OBJ_ID = IPSO_OBJECT_FILLING_LEVEL_SENSOR_ID;

	/* If it exists, restore configuration */
	lwm2m_load(OBJ_ID, instance, CONTAINER_HEIGHT_FILLING_SENSOR_RID,
		   sizeof(uint32_t));
	lwm2m_load(OBJ_ID, instance,
		   HIGH_THRESHOLD_PERCENTAGE_FILLING_SENSOR_RID,
		   sizeof(float64_value_t));
	lwm2m_load(OBJ_ID, instance,
		   LOW_THRESHOLD_PERCENTAGE_FILLING_SENSOR_RID,
		   sizeof(float64_value_t));

	/* Callback is used to save config to nv */
	register_post_write_callback(OBJ_ID, instance,
				     CONTAINER_HEIGHT_FILLING_SENSOR_RID,
				     fill_sensor_write_cb);
	register_post_write_callback(
		OBJ_ID, instance, HIGH_THRESHOLD_PERCENTAGE_FILLING_SENSOR_RID,
		fill_sensor_write_cb);
	register_post_write_callback(
		OBJ_ID, instance, LOW_THRESHOLD_PERCENTAGE_FILLING_SENSOR_RID,
		fill_sensor_write_cb);

	/* Delete unused resources so they don't show up in Cumulocity. */
	lwm2m_delete_resource_inst(OBJ_ID, instance,
				   AVERAGE_FILL_SPEED_FILLING_SENSOR_RID, 0);
	lwm2m_delete_resource_inst(OBJ_ID, instance,
				   FORECAST_FULL_DATE_FILLING_SENSOR_RID, 0);
	lwm2m_delete_resource_inst(OBJ_ID, instance,
				   FORECAST_EMPTY_DATE_FILLING_SENSOR_RID, 0);
	lwm2m_delete_resource_inst(OBJ_ID, instance,
				   CONTAINER_OUT_OF_LOCATION_FILLING_SENSOR_RID,
				   0);
	lwm2m_delete_resource_inst(OBJ_ID, instance,
				   CONTAINER_OUT_OF_POSITION_FILLING_SENSOR_RID,
				   0);
}

static int register_post_write_callback(uint16_t type, uint16_t instance,
					uint16_t resource,
					lwm2m_engine_set_data_cb_t cb)
{
	char path[CONFIG_LWM2M_PATH_MAX_SIZE];

	snprintk(path, sizeof(path), "%u/%u/%u", type, instance, resource);

	return lwm2m_engine_register_post_write_callback(path, cb);
}

static int fill_sensor_write_cb(uint16_t obj_inst_id, uint16_t res_id,
				uint16_t res_inst_id, uint8_t *data,
				uint16_t data_len, bool last_block,
				size_t total_size)
{
	lwm2m_save(IPSO_OBJECT_FILLING_LEVEL_SENSOR_ID, obj_inst_id, res_id,
		   data, data_len);

	return 0;
}
